#include <iostream>
#include <vector>
#include <mutex>
#include <dlfcn.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "mediaLibrary.hpp"
#include "opus/include/opus.h"
#include "codec/codec_api.h"
#include "uvgRTP/include/uvgrtp/lib.hh"

using namespace std;
using namespace uvgrtp;

void *handle;
string libraryPath;

// RTP送受信用変数(デフォルト)
char peerAddress[16] = "127.0.0.1";
uint16_t myTcpPort = 30001;
uint16_t peerTcpPort = 30000;
uint16_t myVideoPort = 30006;
uint16_t peerVideoPort = 30002;
uint16_t myAudioPort = 30004;
uint16_t peerAudioPort = 30008;

// TCP用ソケット
int server_fd, new_socket;

// 送信用ストリーム
context sendVideoCtx;
session *sendVideoSess;
media_stream *sendVideoStrm;

context sendAudioCtx;
session *sendAudioSess;
media_stream *sendAudioStrm;

// 受信用ストリーム
context receiveVideoCtx;
session *receiveVideoSess;
media_stream *receiveVideoStrm;

context receiveAudioCtx;
session *receiveAudioSess;
media_stream *receiveAudioStrm;

// 映像エンコード用変数
ISVCEncoder *videoEncoder = nullptr;
SEncParamBase videoEncParam;
uint16_t width = 1920;
uint16_t height = 1080;
uint16_t frameRate = 30;
int bitrate = 5000000;
uint16_t mtuSize = 1400;
int send_flags = RCE_PACE_FRAGMENT_SENDING | RCE_SEND_ONLY;
int videoFormat = videoFormatI420;

size_t videoEncodeBufferSize = width * height * 3 / 2;

// yuvなら0,RGBなら1
int inputVideoFormat;

// 映像デコード用変数
ISVCDecoder *videoDecoder = nullptr;

// 音声エンコード用変数
uint16_t SAMPLE_RATE = 48000;
uint16_t CHANNELS = 1;
uint16_t FRAME_SIZE = 960;
uint16_t MAX_PACKET_SIZE = 1400;

OpusEncoder *audioEncoder = nullptr;

// 音声デコード用変数
OpusDecoder *audioDecoder = nullptr;

// プラットフォーム接続用
CallbackType debugCallback = nullptr;
ReceiveCallbackType receiveVideoCallback = nullptr;
ReceiveCallbackType receiveAudioCallback = nullptr;

/* =======================================
コーデックのパラメータ設定用関数
RTPのパラメータ設定用関数
コールバック関数 / ライブラリパスの設定用の関数
======================================= */

// コールバック関数の設定用関数
void setMediaCallback(CallbackType debug, ReceiveCallbackType receiveVideo, ReceiveCallbackType receiveAudio)
{
    debugCallback = debug;
    receiveVideoCallback = receiveVideo;
    receiveAudioCallback = receiveAudio;

    debugCallback("debugコールバック");
}

// openh264ライブラリのパスを設定する関数
void setLibraryPath(const string &path)
{
    libraryPath = path.c_str();
}

void setAddressAndPort(
    const char *address,
    int mytcpport,
    int peertcpport,
    int myvideoport,
    int peervideoport,
    int myaudioport,
    int peeraudioport)
{
    strncpy(peerAddress, address, sizeof(peerAddress) - 1);
    peerAddress[sizeof(peerAddress) - 1] = '\0';

    myTcpPort = mytcpport;
    peerTcpPort = peertcpport;
    myVideoPort = myvideoport;
    peerVideoPort = peervideoport;
    myAudioPort = myaudioport;
    peerAudioPort = peeraudioport;
}

/* =======================================
    TCPで接続確認をおこなう関数
======================================= */

// TCPを初期化して接続を試みる関数
// Senderが接続先（Receiver）を見つけてから通信開始をする
bool initializeTcpSender()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    const char *message = "Hello from Sender";

    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(peerTcpPort); // TCPサーバーのポート番号

    // 受信側サーバーのアドレスを設定
    if (inet_pton(AF_INET, peerAddress, &serv_addr.sin_addr) <= 0)
    {
        return false;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        debugCallback("接続先が見つかりませんでした");
        close(sock);
        return false;
    }

    debugCallback("接続完了");
    // メッセージを送信し、応答を待つ
    send(sock, message, strlen(message), 0);
    read(sock, buffer, 1024);

    // ソケットを閉じる
    close(sock);
    return true;
}

// TCPサーバーを初期化して接続を待ち受ける関数
bool initializeTcpReceiver()
{
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    const char *message = "Hello from Receiver";

    // ソケットを作成
    if ((server_fd = ::socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        return false;
    }

    // ソケットにオプションを設定
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(myTcpPort);

    // ソケットをポートにバインド
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        return false;
    }

    // 接続を待ち受ける
    if (listen(server_fd, 3) < 0)
    {
        return false;
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        return false;
    }

    // クライアントからのメッセージを受信し、応答を送信
    read(new_socket, buffer, 1024);
    send(new_socket, message, strlen(message), 0);

    // ソケットを閉じる
    close(new_socket);
    close(server_fd);

    return true;
}

void closeTcpSocket()
{
    close(new_socket);
    close(server_fd);
}

/**
 * ビデオ送受信用関数
 * 送信側流れ
 * 1. 各プラットフォームから映像データをもらう
 * 2. openh264でフレームごとにエンコード
 * 3. RTPでフラグメント化して送信
 *
 * 受信側流れ
 * 4. RTPでフラグメント化したデータを受信
 * 5. openh264でフレームごとにデコード
 * 6. 各プラットフォームへ渡す
 **/

/* =======================================
                送信側コード
======================================= */

// ビデオエンコーダの初期化関数
int initEncodeVideoData(int videoFormatNum)
{
    handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle)
    {
        debugCallback("DLOpenエラー");
        return -1;
    }

    typedef int (*CreateEncoderFunc)(ISVCEncoder **);
    CreateEncoderFunc createEncoder = (CreateEncoderFunc)dlsym(handle, "WelsCreateSVCEncoder");
    if (createEncoder(&videoEncoder) != 0 || videoEncoder == nullptr)
    {
        debugCallback("ビデオエンコーダー作成エラー");
        return -1;
    }

    inputVideoFormat = videoFormatNum;

    // エンコーダの初期化パラメータ
    memset(&videoEncParam, 0, sizeof(SEncParamBase));
    videoEncParam.iUsageType = CAMERA_VIDEO_REAL_TIME;
    videoEncParam.fMaxFrameRate = frameRate;
    videoEncParam.iPicWidth = width;
    videoEncParam.iPicHeight = height;
    videoEncParam.iTargetBitrate = bitrate;
    if (videoEncoder->Initialize(&videoEncParam) != 0)
    {
        debugCallback("ビデオエンコーダー作成エラー");
        return -1;
    }

    videoEncoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

    // uvgRTPの初期化
    sendVideoSess = sendVideoCtx.create_session(peerAddress);
    sendVideoStrm = sendVideoSess->create_stream(peerVideoPort, RTP_FORMAT_H264, send_flags);
    sendVideoStrm->configure_ctx(RCC_MTU_SIZE, mtuSize);
    if (sendVideoSess == NULL || sendVideoStrm == NULL)
    {
        debugCallback("ビデオエンコーダー作成エラー2");
        return -1;
    }

    return 0;
}

// ビデオデータのエンコード関数
void encodeVideoData(unsigned char *inputData, int length)
{
    cv::Mat frame(height, width, CV_8UC4, inputData);
    cv::Mat yuv;

    if (inputVideoFormat == 1)
    {
        // RGBフォーマットの場合
        cv::cvtColor(frame, yuv, cv::COLOR_RGBA2YUV_I420);
    }

    SSourcePicture pic;
    memset(&pic, 0, sizeof(SSourcePicture));
    pic.iPicWidth = width;
    pic.iPicHeight = height;
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = width;
    pic.iStride[1] = pic.iStride[2] = width / 2;
    pic.pData[0] = yuv.data;
    pic.pData[1] = yuv.data + width * height;
    pic.pData[2] = pic.pData[1] + (width * height) / 4;

    SFrameBSInfo info;
    memset(&info, 0, sizeof(SFrameBSInfo));
    if (videoEncoder->EncodeFrame(&pic, &info) != 0)
    {
        debugCallback("エンコードに失敗");
        return;
    }

    // エンコードされたデータをRTPで送信
    for (int layer = 0; layer < info.iLayerNum; ++layer)
    {
        int layerSize = 0;
        SLayerBSInfo &layerInfo = info.sLayerInfo[layer];

        for (int nal = 0; nal < layerInfo.iNalCount; ++nal)
        {
            if (layerInfo.pBsBuf == nullptr)
            {
                debugCallback("エンコードされたデータがnullptrです");
                return;
            }

            if (layerInfo.pNalLengthInByte[nal] + layerSize > videoEncodeBufferSize)
            {
                // bufferSize はエンコーディングバッファのサイズ
                debugCallback("NALユニットのサイズがバッファサイズを超えています");
                return;
            }

            if (sendVideoStrm->push_frame(layerInfo.pBsBuf + layerSize, layerInfo.pNalLengthInByte[nal], RTP_NO_H26X_SCL) != RTP_OK)
            {
                debugCallback("RTPフレームの送信に失敗しました");
                return;
            }

            layerSize += layerInfo.pNalLengthInByte[nal];
        }
    }
}

void initEncodeAudioData()
{
    // Opusエンコーダーの初期化
    int opusErr;
    audioEncoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusErr);
    if (opusErr != OPUS_OK)
    {
        debugCallback("Opusエンコーダの初期化に失敗");
        return;
    }

    // uvgRTPの初期化
    sendAudioSess = sendAudioCtx.create_session(peerAddress);
    sendAudioStrm = sendAudioSess->create_stream(peerAudioPort, RTP_FORMAT_OPUS, RCE_SEND_ONLY);
    sendAudioStrm->configure_ctx(RCC_MTU_SIZE, 1400);
}

// オーディオデータのエンコードと送信を行う関数
void encodeAudioData(const unsigned char *audioData, int length)
{
    // オーディオデータのエンコード
    vector<unsigned char> opusData(MAX_PACKET_SIZE);
    int numBytes = opus_encode(audioEncoder, reinterpret_cast<const opus_int16 *>(audioData), FRAME_SIZE, opusData.data(), opusData.size());
    if (numBytes < 0)
    {
        debugCallback("Opusエンコードに失敗");
    }
    else
    {
        // エンコードされたデータをRTPで送信
        sendAudioStrm->push_frame(opusData.data(), numBytes, RTP_NO_FLAGS);
    }
}

// エンコーダのクリーンアップ関数
void destroyEncoder()
{
    // ビデオエンコーダとビデオ用uvgRTPのリソース解放
    videoEncoder->Uninitialize();
    typedef void (*DestroyEncoderFunc)(ISVCEncoder *);
    DestroyEncoderFunc destroyVideoEncoder = (DestroyEncoderFunc)dlsym(handle, "WelsDestroySVCEncoder");
    destroyVideoEncoder(videoEncoder);
    dlclose(handle);

    sendVideoSess->destroy_stream(sendVideoStrm);
    sendVideoCtx.destroy_session(sendVideoSess);

    // エンコーダとuvgRTPのリソース解放
    opus_encoder_destroy(audioEncoder);
    sendAudioSess->destroy_stream(sendAudioStrm);
    sendAudioCtx.destroy_session(sendAudioSess);
}

/* =======================================
                受信側コード
======================================= */

// 映像デコード準備関数
void initDecodeVideoData()
{
    handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle)
    {
        debugCallback("DLOpenエラー");
        return;
    }

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");

    if (createDecoder(&videoDecoder) != 0 || videoDecoder == nullptr)
    {
        debugCallback("デコーダの作成に失敗");
        return;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if (videoDecoder->Initialize(&decParam) != 0)
    {
        debugCallback("デコーダの初期化に失敗");
        return;
    }

    receiveVideoSess = receiveVideoCtx.create_session(peerAddress);
    receiveVideoStrm = receiveVideoSess->create_stream(myVideoPort, RTP_FORMAT_H264, RCE_RECEIVE_ONLY);
    debugCallback("デコーダの設定成功");
}

// 映像デコード関数
void receiveAndDecodeVideoData()
{
    auto frame = receiveVideoStrm->pull_frame(1000);
    if (!frame)
    {
        debugCallback("RTPフレームの受信に失敗しました");
        return;
    }
    // デコード処理
    SBufferInfo bufInfo;
    memset(&bufInfo, 0, sizeof(SBufferInfo));
    unsigned char *pData[3] = {nullptr, nullptr, nullptr};

    int ret = videoDecoder->DecodeFrameNoDelay(frame->payload, frame->payload_len, pData, &bufInfo);
    if (ret == 0 && bufInfo.iBufferStatus == 1)
    {
        int width = bufInfo.UsrData.sSystemBuffer.iWidth;
        int height = bufInfo.UsrData.sSystemBuffer.iHeight;
        int yStride = bufInfo.UsrData.sSystemBuffer.iStride[0];
        int uvStride = bufInfo.UsrData.sSystemBuffer.iStride[1];

        // Y成分のMatオブジェクトを作成
        cv::Mat yMat(height, width, CV_8UC1, pData[0], yStride);

        // U成分とV成分のMatオブジェクトを作成
        cv::Mat uMat(height / 2, width / 2, CV_8UC1, pData[1], uvStride);
        cv::Mat vMat(height / 2, width / 2, CV_8UC1, pData[2], uvStride);

        // YUV420p形式に合わせてY, U, Vデータを配置
        cv::Mat yuvImg(height + height / 2, width, CV_8UC1);
        yMat.copyTo(yuvImg(cv::Rect(0, 0, width, height)));

        // U成分とV成分の配置を修正
        // U成分を配置
        cv::Mat uPart(uMat.size(), CV_8UC1, yuvImg.data + height * width);
        uMat.copyTo(uPart);

        // V成分を配置
        cv::Mat vPart(vMat.size(), CV_8UC1, yuvImg.data + height * width + (height / 2) * (width / 2));
        vMat.copyTo(vPart);

        // YUVからRGBへ変換
        cv::Mat rgbImg;
        cvtColor(yuvImg, rgbImg, cv::COLOR_YUV2RGBA_I420);

        size_t size = rgbImg.total() * rgbImg.elemSize();
        unsigned char *frameData = new unsigned char[size]; // ヒープにデータを確保
        std::memcpy(frameData, rgbImg.data, size);

        receiveVideoCallback(frameData, size, 3);
        frame::dealloc_frame(frame);

        delete[] frameData;
    }
    else
    {
        debugCallback("デコードに失敗 or 1000msが経過");
    }
}

// 音声デコード準備関数
void initDecodeAudioData(unsigned char *inputData, int inputLength, opus_int16 *outputData, int outputLength)
{
    int opusErr;
    audioDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);
    if (opusErr != OPUS_OK)
    {
        debugCallback("opusエンコーダの作成に失敗");
        return;
    }

    receiveAudioSess = receiveAudioCtx.create_session(peerAddress);
    receiveAudioStrm = receiveAudioSess->create_stream(myAudioPort, RTP_FORMAT_OPUS, RCE_RECEIVE_ONLY);
}

// 音声デコード・関数
void decodeAudioData()
{
    receiveVideoCallback(audioData, size, 3);
}

void destroyDecoder()
{
    videoDecoder->Uninitialize();

    if (receiveVideoStrm)
        receiveVideoSess->destroy_stream(receiveVideoStrm);

    if (receiveVideoSess)
        receiveVideoCtx.destroy_session(receiveVideoSess);

    // デコーダの解放
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(videoDecoder);

    dlclose(handle);

    debugCallback("セッションとストリームの解放");
}

// セットアップテスト用
void test()
{
    debugCallback(libraryPath.c_str());
    // OpenH264デコーダの初期化
    handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle)
    {
        debugCallback("DLOpenエラー");
        return;
    }

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");

    if (createDecoder(&videoDecoder) != 0 || videoDecoder == nullptr)
    {
        debugCallback("VideoDecoder作成エラー");
        return;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if (videoDecoder->Initialize(&decParam) != 0)
    {
        debugCallback("VideoDecoderイニシャライズエラー");
        return;
    }

    int opusErr;
    audioDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);
    if (opusErr != OPUS_OK)
    {
        debugCallback("Audioデコーダーエラー");
        return;
    }

    if (debugCallback != nullptr)
    {
        debugCallback("テスト関数 最後まで呼び出し成功");
    }
}

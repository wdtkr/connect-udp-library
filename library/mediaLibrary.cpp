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

// RTP送受信用変数
char PEER_ADDRESS[] = "127.0.0.1";
uint16_t VIDEO_PORT = 30002;
uint16_t AUDIO_PORT = 30004;

// TCP用ソケット
int server_fd, new_socket;

// 送信用ストリーム
context sendCtx;
session *sendSess;
media_stream *sendStrm;

// 受信用ストリーム
context receiveCtx;
session *receiveSess;
media_stream *receiveStrm;

// 映像エンコード用変数
void *handle;
string libraryPath;

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

// 音声デコード用変数
OpusDecoder *audioDecoder = nullptr;

// プラットフォーム接続用
CallbackType debug_callback = nullptr;
ReceiveCallbackType receive_callback = nullptr;

/* =======================================
コーデックのパラメータ設定用関数
RTPのパラメータ設定用関数
コールバック関数 / ライブラリパスの設定用の関数
======================================= */

// コールバック関数の設定用関数
void setCallback(CallbackType debug, ReceiveCallbackType receive)
{
    debug_callback = debug;
    receive_callback = receive;

    debug_callback("debugコールバック");
}

// openh264ライブラリのパスを設定する関数
void setLibraryPath(const string &path)
{
    libraryPath = path.c_str();
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
    serv_addr.sin_port = htons(30000); // TCPサーバーのポート番号

    // 受信側サーバーのアドレスを設定
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        return false;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        debug_callback("接続先が見つかりませんでした");
        return false;
    }

    debug_callback("接続完了");
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
    address.sin_port = htons(30000); // ポート番号

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
        debug_callback("DLOpenエラー");
        return -1;
    }

    typedef int (*CreateEncoderFunc)(ISVCEncoder **);
    CreateEncoderFunc createEncoder = (CreateEncoderFunc)dlsym(handle, "WelsCreateSVCEncoder");
    if (createEncoder(&videoEncoder) != 0 || videoEncoder == nullptr)
    {
        debug_callback("ビデオエンコーダー作成エラー");
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
        debug_callback("ビデオエンコーダー作成エラー");
        return -1;
    }

    videoEncoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

    // uvgRTPの初期化
    sendSess = sendCtx.create_session(PEER_ADDRESS);
    sendStrm = sendSess->create_stream(VIDEO_PORT, RTP_FORMAT_H264, send_flags);
    sendStrm->configure_ctx(RCC_MTU_SIZE, mtuSize);
    if (sendSess == NULL || sendStrm == NULL)
    {
        debug_callback("ビデオエンコーダー作成エラー2");
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
        debug_callback("エンコードに失敗");
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
                debug_callback("エンコードされたデータがnullptrです");
                return;
            }

            if (layerInfo.pNalLengthInByte[nal] + layerSize > videoEncodeBufferSize)
            {
                // bufferSize はエンコーディングバッファのサイズ
                debug_callback("NALユニットのサイズがバッファサイズを超えています");
                return;
            }

            if (sendStrm->push_frame(layerInfo.pBsBuf + layerSize, layerInfo.pNalLengthInByte[nal], RTP_NO_H26X_SCL) != RTP_OK)
            {
                debug_callback("RTPフレームの送信に失敗しました");
                return;
            }

            layerSize += layerInfo.pNalLengthInByte[nal];
        }
    }
}

// オーディオデータのエンコード関数
void EncodeAudioData(const vector<opus_int16> &inputData)
{
    //
}

// エンコーダのクリーンアップ関数
void destroyEncoder()
{
    // エンコーダとuvgRTPのリソース解放
    videoEncoder->Uninitialize();
    typedef void (*DestroyEncoderFunc)(ISVCEncoder *);
    DestroyEncoderFunc destroyVideoEncoder = (DestroyEncoderFunc)dlsym(handle, "WelsDestroySVCEncoder");
    destroyVideoEncoder(videoEncoder);
    dlclose(handle);
    sendSess->destroy_stream(sendStrm);
    sendCtx.destroy_session(sendSess);
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
        debug_callback("DLOpenエラー");
        return;
    }

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");

    if (createDecoder(&videoDecoder) != 0 || videoDecoder == nullptr)
    {
        debug_callback("デコーダの作成に失敗");
        return;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if (videoDecoder->Initialize(&decParam) != 0)
    {
        debug_callback("デコーダの初期化に失敗");
        return;
    }

    receiveSess = receiveCtx.create_session(PEER_ADDRESS);
    receiveStrm = receiveSess->create_stream(VIDEO_PORT, RTP_FORMAT_H264, RCE_RECEIVE_ONLY);
    debug_callback("デコーダの設定成功");
}

// 映像デコード関数
void receiveAndDecodeVideoData()
{
    auto frame = receiveStrm->pull_frame(1000);
    if (!frame)
    {
        debug_callback("RTPフレームの受信に失敗しました");
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

        receive_callback(frameData, size, 3);
        frame::dealloc_frame(frame);
    }
    else
    {
        debug_callback("デコードに失敗 or 1000msが経過");
    }
}

// 音声デコード準備関数
int preDecodeAudioData(unsigned char *inputData, int inputLength, opus_int16 *outputData, int outputLength)
{
    int opusErr;
    audioDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);
    if (opusErr != OPUS_OK)
    {
        cerr << "Opusデコーダの初期化に失敗" << endl;
        return -1;
    }
}

// 音声デコード・関数
int decodeAudioData(unsigned char *inputData, int inputLength, opus_int16 *outputData, int outputLength)
{
    int frameSize = opus_decode(audioDecoder, inputData, inputLength, outputData, FRAME_SIZE, 0);
    if (frameSize < 0)
    {
        cerr << "Opusデコードに失敗: " << opus_strerror(frameSize) << endl;
        return -1;
    }

    // デコード成功
    return frameSize;
}

void destroyDecoder()
{
    videoDecoder->Uninitialize();

    if (receiveStrm)
    {
        receiveSess->destroy_stream(receiveStrm);
    }

    if (receiveSess)
    {
        receiveCtx.destroy_session(receiveSess);
    }

    // デコーダの解放
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(videoDecoder);

    dlclose(handle);

    debug_callback("セッションとストリームの解放");
}

// セットアップテスト用
void test()
{
    debug_callback(libraryPath.c_str());
    // OpenH264デコーダの初期化
    handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle)
    {
        debug_callback("DLOpenエラー");
        return;
    }

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");

    if (createDecoder(&videoDecoder) != 0 || videoDecoder == nullptr)
    {
        debug_callback("VideoDecoder作成エラー");
        return;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if (videoDecoder->Initialize(&decParam) != 0)
    {
        debug_callback("VideoDecoderイニシャライズエラー");
        return;
    }

    int opusErr;
    audioDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);
    if (opusErr != OPUS_OK)
    {
        debug_callback("Audioデコーダーエラー");
        return;
    }

    if (debug_callback != nullptr)
    {
        debug_callback("テスト関数 最後まで呼び出し成功");
    }
}

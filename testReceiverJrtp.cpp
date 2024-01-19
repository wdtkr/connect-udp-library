#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opus.h>
#include <portaudio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpmemorymanager.h"

using namespace std;
using namespace cv;
using namespace jrtplib;

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960

// グローバル変数
constexpr char LOCAL_ADDRESS[] = "127.0.0.1";
// ビデオ
constexpr uint16_t VIDEO_LOCAL_PORT = 30002;
// オーディオ
constexpr uint16_t AUDIO_LOCAL_PORT = 30004;

std::queue<std::vector<opus_int16>> playbackQueue;
std::mutex queueMutex;
std::condition_variable conditionVariable;
std::atomic<bool> stopSignal(false);

// 再生のコールバック関数
static int playbackCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    lock_guard<mutex> lock(queueMutex);

    if (!playbackQueue.empty())
    {
        auto &front = playbackQueue.front();
        copy(front.begin(), front.end(), static_cast<opus_int16 *>(outputBuffer));
        playbackQueue.pop();
    }
    else
    {
        fill_n(static_cast<opus_int16 *>(outputBuffer), framesPerBuffer * CHANNELS, 0);
    }

    return paContinue;
}

// TCPサーバーを初期化して接続を待ち受ける関数
bool initializeTcpServer()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    const char *message = "Hello from Receiver";

    // ソケットを作成
    if ((server_fd = ::socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket failed" << std::endl;
        return false;
    }

    // ソケットにオプションを設定
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "Setsockopt failed" << std::endl;
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(30000); // ポート番号

    std::cout << "Test1" << std::endl;

    // ソケットをポートにバインド
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        std::cerr << "Bind failed" << std::endl;
        return false;
    }

    std::cout << "Test2" << std::endl;

    // 接続を待ち受ける
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        return false;
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return false;
    }

    std::cout << "Test3" << std::endl;

    // クライアントからのメッセージを受信し、応答を送信
    read(new_socket, buffer, 1024);
    std::cout << "Client: " << buffer << std::endl;
    send(new_socket, message, strlen(message), 0);

    // ソケットを閉じる
    close(new_socket);
    close(server_fd);

    return true;
}

// オーディオ受信と再生を行う関数
void receiveAndPlayAudio()
{
    // Opusデコーダの初期化
    int opusErr;
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);

    // JRTPLIBの初期化
    RTPSession session;
    RTPSessionParams sessionParams;
    RTPUDPv4TransmissionParams transParams;

    sessionParams.SetOwnTimestampUnit(1.0 / 48000.0); // Opusのタイムスタンプ単位
    transParams.SetPortbase(AUDIO_LOCAL_PORT);
    session.Create(sessionParams, &transParams);

    while (!stopSignal)
    {
        session.BeginDataAccess();
        if (session.GotoFirstSourceWithData())
        {
            do
            {
                RTPPacket *packet;
                while ((packet = session.GetNextPacket()) != nullptr)
                {
                    // ペイロードを取得し、デコーダに渡す
                    unsigned char *payload = packet->GetPayloadData();
                    int payloadLength = packet->GetPayloadLength();

                    vector<opus_int16> decoded(FRAME_SIZE * CHANNELS);
                    int frameSize = opus_decode(decoder, payload, payloadLength, decoded.data(), FRAME_SIZE, 0);

                    if (frameSize >= 0)
                    {
                        lock_guard<mutex> lock(queueMutex);
                        playbackQueue.push(std::move(decoded));
                        conditionVariable.notify_one();
                    }

                    // パケットの解放
                    session.DeletePacket(packet);
                }
            } while (session.GotoNextSourceWithData());
        }
        else
        {
            cout << "音声受信できてないよ" << endl;
        }
        session.EndDataAccess();
    }

    // クリーンアップ
    opus_decoder_destroy(decoder);
    session.BYEDestroy(RTPTime(1, 0), 0, 0);
}

int main()
{
    // ビデオ受信とデコードを行う処理（メインスレッド）
    RTPSession videoSession;
    RTPSessionParams videoSessionParams;
    RTPUDPv4TransmissionParams videoTransParams;

    videoSessionParams.SetOwnTimestampUnit(1.0 / 90000.0);
    videoTransParams.SetPortbase(VIDEO_LOCAL_PORT);
    videoSession.Create(videoSessionParams, &videoTransParams);

    // OpenH264デコーダの初期化
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");
    ISVCDecoder *decoder = nullptr;
    if (createDecoder(&decoder) != 0 || decoder == nullptr)
    {
        cerr << "デコーダの作成に失敗" << endl;
        return -1;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    decoder->Initialize(&decParam);

    namedWindow("Decoded Frame", WINDOW_AUTOSIZE);

    // PortAudioの初期化
    Pa_Initialize();
    PaStream *playbackStream;
    Pa_OpenDefaultStream(&playbackStream, 0, CHANNELS, paInt16, SAMPLE_RATE, FRAME_SIZE, playbackCallback, nullptr);
    Pa_StartStream(playbackStream);

    // // TCPで受信待機
    if (!initializeTcpServer())
    {
        std::cerr << "Failed to initialize TCP server" << std::endl;
        return -1;
    }

    // オーディオ受信スレッドの起動
    thread audioThread(receiveAndPlayAudio);

    // メインスレッドでビデオ受信とデコード
    while (!stopSignal)
    {
        videoSession.BeginDataAccess();
        if (videoSession.GotoFirstSourceWithData())
        {
            do
            {
                RTPPacket *packet;
                while ((packet = videoSession.GetNextPacket()) != nullptr)
                {
                    // ペイロードを取得し、デコーダに渡す
                    unsigned char *payload = packet->GetPayloadData();
                    int payloadLength = packet->GetPayloadLength();

                    // デコード処理
                    SBufferInfo bufInfo;
                    memset(&bufInfo, 0, sizeof(SBufferInfo));
                    unsigned char *pData[3] = {nullptr, nullptr, nullptr};

                    int ret = decoder->DecodeFrameNoDelay(payload, payloadLength, pData, &bufInfo);
                    if (ret == 0 && bufInfo.iBufferStatus == 1)
                    {
                        cout << "エラーじゃないよ。" << endl;
                        int width = bufInfo.UsrData.sSystemBuffer.iWidth;
                        int height = bufInfo.UsrData.sSystemBuffer.iHeight;
                        int yStride = bufInfo.UsrData.sSystemBuffer.iStride[0];
                        int uvStride = bufInfo.UsrData.sSystemBuffer.iStride[1];

                        // Y成分のMatオブジェクトを作成
                        Mat yMat(height, width, CV_8UC1, pData[0], yStride);

                        // U成分とV成分のMatオブジェクトを作成
                        Mat uMat(height / 2, width / 2, CV_8UC1, pData[1], uvStride);
                        Mat vMat(height / 2, width / 2, CV_8UC1, pData[2], uvStride);

                        // YUV420p形式に合わせてY, U, Vデータを配置
                        Mat yuvImg(height + height / 2, width, CV_8UC1);
                        yMat.copyTo(yuvImg(Rect(0, 0, width, height)));

                        // U成分とV成分の配置を修正
                        // U成分を配置
                        Mat uPart(uMat.size(), CV_8UC1, yuvImg.data + height * width);
                        uMat.copyTo(uPart);

                        // V成分を配置
                        Mat vPart(vMat.size(), CV_8UC1, yuvImg.data + height * width + (height / 2) * (width / 2));
                        vMat.copyTo(vPart);

                        // YUVからRGBへ変換
                        Mat rgbImg;
                        cvtColor(yuvImg, rgbImg, COLOR_YUV2BGR_I420);

                        imshow("Decoded Frame", rgbImg);
                    }
                    else
                    {
                        cout << "エラーです。" << endl;
                    }

                    int key = waitKey(10);
                    if (key == 'q' || key == 'Q')
                    {
                        stopSignal = true;
                        // break;
                    }

                    videoSession.DeletePacket(packet);
                }
            } while (videoSession.GotoNextSourceWithData());
        }
        else
        {
            cout << "カメラ受信できてないよ" << endl;
        }

        videoSession.EndDataAccess();
    }

    videoSession.BYEDestroy(RTPTime(1, 0), 0, 0);

    conditionVariable.notify_one();
    // オーディオスレッドの終了待ち
    audioThread.join();

    // 再生用ストリームの停止とクリーンアップ
    Pa_StopStream(playbackStream);
    Pa_CloseStream(playbackStream);
    Pa_Terminate();

    decoder->Uninitialize();

    // デコーダの解放
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(decoder);

    dlclose(handle);
    return 0;
}
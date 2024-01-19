#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opus.h>
#include <portaudio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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

// グローバル変数と定数
constexpr char REMOTE_ADDRESS[] = "127.0.0.1";

// ビデオ
constexpr uint16_t VIDEO_REMOTE_PORT = 30002;
const int width = 1920;
const int height = 1080;
const int frameRate = 30;
const int bitrate = 5000000;
const int mtuSize = 1400;
int videoFormat = videoFormatI420;
// フレームのための推定バッファサイズ
size_t bufferSize = width * height * 3 / 2;

// オーディオ
constexpr uint16_t AUDIO_REMOTE_PORT = 30004;
constexpr int SAMPLE_RATE = 48000;
constexpr int CHANNELS = 1;
constexpr int FRAME_SIZE = 960;
constexpr int MAX_PACKET_SIZE = 1500;

mutex g_mutex;
queue<vector<opus_int16>> sampleQueue;
mutex queueMutex;
condition_variable conditionVariable;

bool stopSignal = false;

// データをMTUサイズに合わせて分割する関数
vector<vector<uint8_t>> splitData(const uint8_t *data, size_t size, size_t mtuSize) {
    vector<vector<uint8_t>> packets;
    size_t offset = 0;

    while (size > offset) {
        size_t chunkSize = min(size - offset, mtuSize);
        packets.emplace_back(data + offset, data + offset + chunkSize);
        offset += chunkSize;
    }

    return packets;
}

// 録音のコールバック関数
static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    if (inputBuffer != NULL)
    {
        const opus_int16 *input = static_cast<const opus_int16 *>(inputBuffer);
        vector<opus_int16> samples(input, input + framesPerBuffer * CHANNELS);

        lock_guard<mutex> lock(queueMutex);
        sampleQueue.push(samples);
        conditionVariable.notify_one();
    }
    return paContinue;
}

// TCPクライアントを初期化して接続を試みる関数
bool initializeTcpClient()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    const char *message = "Hello from Sender";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "Socket creation error" << std::endl;
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(30000); // TCPサーバーのポート番号

    // 受信側サーバーのアドレスを設定
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cout << "Invalid address / Address not supported" << std::endl;
        return false;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "Connection failed" << std::endl;
        return false;
    }

    std::cout << "Connected to server successfully" << std::endl; // 接続成功

    // メッセージを送信し、応答を待つ
    send(sock, message, strlen(message), 0);
    std::cout << "Hello message sent" << std::endl;
    read(sock, buffer, 1024);
    std::cout << "Server: " << buffer << std::endl;

    // ソケットを閉じる
    close(sock);
    return true;
}

void sendVideo()
{
    // OpenCVの初期化
    VideoCapture cap(0);
    if (!cap.isOpened())
    {
        cerr << "カメラが開けません" << endl;
        return;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, height);

    // OpenH264エンコーダの初期化
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return;
    }

    typedef int (*CreateEncoderFunc)(ISVCEncoder **);
    CreateEncoderFunc createEncoder = (CreateEncoderFunc)dlsym(handle, "WelsCreateSVCEncoder");
    ISVCEncoder *encoder = nullptr;
    if (createEncoder(&encoder) != 0 || encoder == nullptr)
    {
        cerr << "エンコーダの作成に失敗" << endl;
        return;
    }

    // エンコーダの初期化パラメータ
    SEncParamBase param;
    memset(&param, 0, sizeof(SEncParamBase));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME;
    param.fMaxFrameRate = frameRate;
    param.iPicWidth = width;
    param.iPicHeight = height;
    param.iTargetBitrate = bitrate;
    if (encoder->Initialize(&param) != 0)
    {
        cerr << "エンコーダの初期化に失敗" << endl;
        return;
    }

    encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

    // JRTPLIBのセッション
    RTPSession session;
    RTPSessionParams sessionParams;
    RTPUDPv4TransmissionParams transParams;

    sessionParams.SetOwnTimestampUnit(1.0 / 90000.0);
    sessionParams.SetAcceptOwnPackets(true);
    transParams.SetPortbase(30008);

    // RTPSessionのインスタンスに値を設定
    int status;
    status = session.Create(sessionParams, &transParams);
    if (status < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(status) << std::endl;
        cerr << "RTPの初期化に失敗1-1" << endl;
        return;
    }
    RTPIPv4Address addr(ntohl(inet_addr(REMOTE_ADDRESS)), VIDEO_REMOTE_PORT);
    status = session.AddDestination(addr);
    if (status < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(status) << std::endl;
        cerr << "RTPの初期化に失敗1-2" << endl;
        return;
    }

    Mat frame, yuv;
    while (true)
    {
        cap >> frame;
        if (frame.empty())
            break;

        cvtColor(frame, yuv, COLOR_BGR2YUV_I420);

        SSourcePicture pic;
        memset(&pic, 0, sizeof(SSourcePicture));
        pic.iPicWidth = frame.cols;
        pic.iPicHeight = frame.rows;
        pic.iColorFormat = videoFormatI420;
        pic.iStride[0] = frame.cols;
        pic.iStride[1] = pic.iStride[2] = frame.cols / 2;
        pic.pData[0] = yuv.data;
        pic.pData[1] = yuv.data + frame.cols * frame.rows;
        pic.pData[2] = pic.pData[1] + (frame.cols * frame.rows) / 4;

        SFrameBSInfo info;
        memset(&info, 0, sizeof(SFrameBSInfo));
        if (encoder->EncodeFrame(&pic, &info) != 0)
        {
            cerr << "エンコードに失敗" << endl;
            continue;
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
                    cerr << "エンコードされたデータが nullptr です。" << endl;
                    // エラー処理
                }

                if (layerInfo.pNalLengthInByte[nal] + layerSize > bufferSize)
                { // bufferSize はエンコーディングバッファのサイズ
                    cerr << "NALユニットのサイズがバッファサイズを超えています。" << endl;
                    // エラー処理
                }

                session.SendPacket(layerInfo.pBsBuf + layerSize,
                                   layerInfo.pNalLengthInByte[nal],
                                   96, false, 160);

                layerSize += layerInfo.pNalLengthInByte[nal];
            }
        }

        // // エンコードされたデータを分割してRTPで送信
        // for (int layer = 0; layer < info.iLayerNum; ++layer)
        // {
        //     SLayerBSInfo &layerInfo = info.sLayerInfo[layer];
        //     for (int nal = 0; nal < layerInfo.iNalCount; ++nal)
        //     {
        //         vector<vector<uint8_t>> packets = splitData(
        //             layerInfo.pBsBuf, layerInfo.pNalLengthInByte[nal], mtuSize);

        //         for (const auto &packet : packets)
        //         {
        //             session.SendPacket(packet.data(), packet.size(), 96, false, 160);
        //         }
        //     }
        // }
    }

    // エンコーダとuvgRTPのリソース解放
    encoder->Uninitialize();
    typedef void (*DestroyEncoderFunc)(ISVCEncoder *);
    DestroyEncoderFunc destroyEncoder = (DestroyEncoderFunc)dlsym(handle, "WelsDestroySVCEncoder");
    destroyEncoder(encoder);
    dlclose(handle);

    session.BYEDestroy(RTPTime(1, 0), 0, 0);
}

void sendAudio()
{
    // Opusエンコーダーの初期化
    int opusErr;
    OpusEncoder *encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusErr);

    RTPSession session;
    RTPSessionParams sessionParams;
    RTPUDPv4TransmissionParams transParams;

    sessionParams.SetOwnTimestampUnit(1.0 / 48000.0); // Opusのタイムスタンプ単位
    transParams.SetPortbase(30006);
    int status;
    status = session.Create(sessionParams, &transParams);
    if (status < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(status) << std::endl;
        cerr << "RTPの初期化に失敗2-1" << endl;
        return;
    }

    RTPIPv4Address addr(ntohl(inet_addr(REMOTE_ADDRESS)), AUDIO_REMOTE_PORT);
    status = session.AddDestination(addr);
    if (status < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(status) << std::endl;
        cerr << "RTPの初期化に失敗2-2" << endl;
        return;
    }

    while (true)
    {
        unique_lock<mutex> lock(queueMutex);
        conditionVariable.wait(lock, []
                               { return !sampleQueue.empty() || stopSignal; });

        if (stopSignal && sampleQueue.empty())
            break;

        auto samples = sampleQueue.front();
        sampleQueue.pop();
        lock.unlock();

        vector<unsigned char> opusData(MAX_PACKET_SIZE);
        int numBytes = opus_encode(encoder, samples.data(), FRAME_SIZE, opusData.data(), opusData.size());
        session.SendPacket((void *)opusData.data(), numBytes, 97, false, 160);
    }

    // クリーンアップ
    opus_encoder_destroy(encoder);
    session.BYEDestroy(RTPTime(1, 0), 0, 0);
}

int main()
{
    // TCPで送信待機
    while (true)
    {
        if (initializeTcpClient() == true)
        {
            break;
        }
        sleep(1);
    }

    // ビデオ送信スレッドの起動
    thread videoThread(sendVideo);

    // PortAudioの初期化
    Pa_Initialize();
    PaStream *recordStream;
    Pa_OpenDefaultStream(&recordStream, CHANNELS, 0, paInt16, SAMPLE_RATE, FRAME_SIZE, recordCallback, nullptr);
    Pa_StartStream(recordStream);

    // オーディオ送信スレッドの起動
    thread audioThread(sendAudio);

    cout << "Recording and sending... Press Enter to stop." << endl;
    cin.get();
    stopSignal = true;
    conditionVariable.notify_one();

    audioThread.join();

    // 録音用ストリームの停止とクリーンアップ
    Pa_StopStream(recordStream);
    Pa_CloseStream(recordStream);
    Pa_Terminate();

    // スレッドの終了待ち
    videoThread.join();
    audioThread.join();

    return 0;
}

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opus.h>
#include <portaudio.h>
#include <dlfcn.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "/Users/takeru/Documents/GitHub/connect-udp-library/uvgRTP/include/uvgrtp/lib.hh"

using namespace std;
using namespace cv;

// グローバル変数と定数
constexpr char REMOTE_ADDRESS[] = "127.0.0.1";

// ビデオ
constexpr uint16_t VIDEO_REMOTE_PORT = 30002;
const int width = 1920;
const int height = 1080;
const int frameRate = 30;
const int bitrate = 5000000;
const int mtuSize = 1400;
const int send_flags = RCE_PACE_FRAGMENT_SENDING | RCE_SEND_ONLY;
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

void sendVideo()
{
    // OpenCVの初期化
    VideoCapture cap(1);
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

    // uvgRTPの初期化
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(REMOTE_ADDRESS);
    uvgrtp::media_stream *strm = sess->create_stream(VIDEO_REMOTE_PORT, RTP_FORMAT_H264, send_flags);
    strm->configure_ctx(RCC_MTU_SIZE, mtuSize);

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

                if (strm->push_frame(layerInfo.pBsBuf + layerSize, layerInfo.pNalLengthInByte[nal], RTP_NO_H26X_SCL) != RTP_OK)
                {
                    cerr << "RTPフレームの送信に失敗しました" << endl;
                }

                layerSize += layerInfo.pNalLengthInByte[nal];
            }
        }
    }

    // エンコーダとuvgRTPのリソース解放
    encoder->Uninitialize();
    typedef void (*DestroyEncoderFunc)(ISVCEncoder *);
    DestroyEncoderFunc destroyEncoder = (DestroyEncoderFunc)dlsym(handle, "WelsDestroySVCEncoder");
    destroyEncoder(encoder);
    dlclose(handle);

    sess->destroy_stream(strm);
    ctx.destroy_session(sess);
}

void sendAudio()
{
    // Opusエンコーダーの初期化
    int opusErr;
    OpusEncoder *encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusErr);

    // uvgRTPの初期化
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(REMOTE_ADDRESS);
    auto strm = sess->create_stream(AUDIO_REMOTE_PORT, RTP_FORMAT_OPUS, RCE_SEND_ONLY);
    strm->configure_ctx(RCC_MTU_SIZE, 1400);

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
        strm->push_frame(opusData.data(), numBytes, RTP_NO_FLAGS);
    }

    // クリーンアップ
    opus_encoder_destroy(encoder);
    sess->destroy_stream(strm);
    ctx.destroy_session(sess);
}

int main()
{
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

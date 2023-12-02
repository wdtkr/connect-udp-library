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
using namespace uvgrtp;

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

// オーディオ受信と再生を行う関数
void receiveAndPlayAudio()
{
    // Opusデコーダの初期化
    int opusErr;
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);

    // uvgRTPの初期化
    context ctx;
    session *sess = ctx.create_session(LOCAL_ADDRESS);
    auto strm = sess->create_stream(AUDIO_LOCAL_PORT, RTP_FORMAT_OPUS, RCE_RECEIVE_ONLY);

    while (!stopSignal)
    {
        auto frame = strm->pull_frame();

        if (frame && frame->payload_len > 0)
        {
            vector<opus_int16> decoded(FRAME_SIZE * CHANNELS);
            int frameSize = opus_decode(decoder, frame->payload, frame->payload_len, decoded.data(), FRAME_SIZE, 0);

            if (frameSize >= 0)
            {
                lock_guard<mutex> lock(queueMutex);
                playbackQueue.push(std::move(decoded));
                conditionVariable.notify_one();
            }
        }

        delete frame;
    }
    // クリーンアップ
    opus_decoder_destroy(decoder);
    sess->destroy_stream(strm);
    ctx.destroy_session(sess);
}

int main()
{
    // ビデオ受信とデコードを行う処理（メインスレッド）
    // uvgRTPの初期化
    context ctx;
    session *sess = ctx.create_session(LOCAL_ADDRESS);
    auto h264 = sess->create_stream(VIDEO_LOCAL_PORT, RTP_FORMAT_H264, RCE_RECEIVE_ONLY);

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

    // オーディオ受信スレッドの起動
    thread audioThread(receiveAndPlayAudio);

    // メインスレッドでビデオ受信とデコード
    while (!stopSignal)
    {
        auto frame = h264->pull_frame(1000);
        if (!frame)
        {
            cerr << "RTPフレームの受信に失敗しました" << endl;
            continue;
        }
        // デコード処理
        SBufferInfo bufInfo;
        memset(&bufInfo, 0, sizeof(SBufferInfo));
        unsigned char *pData[3] = {nullptr, nullptr, nullptr};

        int ret = decoder->DecodeFrameNoDelay(frame->payload, frame->payload_len, pData, &bufInfo);
        if (ret == 0 && bufInfo.iBufferStatus == 1)
        {
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
            frame::dealloc_frame(frame);
        }

        int key = waitKey(10);
        if (key == 'q' || key == 'Q')
        {
            stopSignal = true;
            // break;
        }
    }

    conditionVariable.notify_one();
    // オーディオスレッドの終了待ち
    audioThread.join();

    // 再生用ストリームの停止とクリーンアップ
    Pa_StopStream(playbackStream);
    Pa_CloseStream(playbackStream);
    Pa_Terminate();

    decoder->Uninitialize();

    if (h264)
    {
        sess->destroy_stream(h264);
    }
    if (sess)
    {
        ctx.destroy_session(sess);
    }

    // デコーダの解放
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(decoder);

    dlclose(handle);
    return 0;
}

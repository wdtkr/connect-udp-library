#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opus.h>
#include <portaudio.h>
#include "/Users/takeru/Documents/GitHub/connect-udp-library/uvgRTP/include/uvgrtp/lib.hh"

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960
#define MAX_PACKET_SIZE 1500

constexpr char REMOTE_ADDRESS[] = "127.0.0.1";
constexpr uint16_t REMOTE_PORT = 30004;

std::queue<std::vector<opus_int16>> sampleQueue;
std::mutex queueMutex;
std::condition_variable cv;
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
        std::vector<opus_int16> samples(input, input + framesPerBuffer * CHANNELS);

        std::lock_guard<std::mutex> lock(queueMutex);
        sampleQueue.push(samples);
        cv.notify_one();
    }
    return paContinue;
}

void encodeAndSend()
{
    // Opusエンコーダーの初期化
    int opusErr;
    OpusEncoder *encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusErr);

    // uvgRTPの初期化
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(REMOTE_ADDRESS);
    auto strm = sess->create_stream(REMOTE_PORT, RTP_FORMAT_OPUS, RCE_SEND_ONLY);
    strm->configure_ctx(RCC_MTU_SIZE, 1400);

    while (true)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, []
                { return !sampleQueue.empty() || stopSignal; });

        if (stopSignal && sampleQueue.empty())
            break;

        auto samples = sampleQueue.front();
        sampleQueue.pop();
        lock.unlock();

        std::vector<unsigned char> opusData(MAX_PACKET_SIZE);
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
    // PortAudioの初期化
    Pa_Initialize();
    PaStream *recordStream;
    Pa_OpenDefaultStream(&recordStream, CHANNELS, 0, paInt16, SAMPLE_RATE, FRAME_SIZE, recordCallback, nullptr);
    Pa_StartStream(recordStream);

    std::thread sendThread(encodeAndSend);

    std::cout << "Recording and sending... Press Enter to stop." << std::endl;
    std::cin.get();
    stopSignal = true;
    cv.notify_one();

    sendThread.join();

    // 録音用ストリームの停止とクリーンアップ
    Pa_StopStream(recordStream);
    Pa_CloseStream(recordStream);
    Pa_Terminate();

    std::cout << "Recording and sending finished." << std::endl;
    return 0;
}

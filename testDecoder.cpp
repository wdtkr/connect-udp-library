#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <opus.h>
#include <portaudio.h>
#include "/Users/takeru/Documents/GitHub/connect-udp-library/uvgRTP/include/uvgrtp/lib.hh"

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960

constexpr char LOCAL_ADDRESS[] = "127.0.0.1";
constexpr uint16_t REMOTE_PORT = 30004;

std::queue<std::vector<opus_int16>> playbackQueue;
std::mutex queueMutex;
std::condition_variable cv;
bool stopSignal = false;

// 再生のコールバック関数
static int playbackCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    if (!playbackQueue.empty())
    {
        auto &front = playbackQueue.front();
        std::copy(front.begin(), front.end(), static_cast<opus_int16 *>(outputBuffer));
        playbackQueue.pop();
    }
    else
    {
        std::fill_n(static_cast<opus_int16 *>(outputBuffer), framesPerBuffer * CHANNELS, 0);
    }

    return paContinue;
}

void receiveAndDecode()
{
    // Opusデコーダの初期化
    int opusErr;
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);

    // uvgRTPの初期化
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(LOCAL_ADDRESS);
    auto strm = sess->create_stream(REMOTE_PORT, RTP_FORMAT_OPUS, RCE_RECEIVE_ONLY);

    while (!stopSignal)
    {
        auto frame = strm->pull_frame();

        if (frame && frame->payload_len > 0)
        {
            std::vector<opus_int16> decoded(FRAME_SIZE * CHANNELS);
            int frameSize = opus_decode(decoder, frame->payload, frame->payload_len, decoded.data(), FRAME_SIZE, 0);

            if (frameSize >= 0)
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                playbackQueue.push(std::move(decoded));
                cv.notify_one();
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
    // PortAudioの初期化
    Pa_Initialize();
    PaStream *playbackStream;
    Pa_OpenDefaultStream(&playbackStream, 0, CHANNELS, paInt16, SAMPLE_RATE, FRAME_SIZE, playbackCallback, nullptr);
    Pa_StartStream(playbackStream);

    std::thread receiveThread(receiveAndDecode);

    std::cout << "Receiving and playing... Press Enter to stop." << std::endl;
    std::cin.get();
    stopSignal = true;
    cv.notify_one();

    receiveThread.join();

    // 再生用ストリームの停止とクリーンアップ
    Pa_StopStream(playbackStream);
    Pa_CloseStream(playbackStream);
    Pa_Terminate();

    std::cout << "Receiving and playing finished." << std::endl;
    return 0;
}

#include <iostream>
#include <vector>
#include <fstream>
#include <opus.h>
#include <portaudio.h>

#define SAMPLE_RATE 48000 // サンプリングレート (48kHz)
#define CHANNELS 1        // チャンネル数 (モノラル)
#define FRAME_SIZE 960    // Opusフレームサイズ

using namespace std;

// 再生のコールバック関数
static int playCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData)
{
    vector<opus_int16> *decodedSamples = (vector<opus_int16> *)userData;
    opus_int16 *output = (opus_int16 *)outputBuffer;

    if (decodedSamples->empty())
    {
        return paComplete; // デコードされたサンプルがなくなったら終了
    }

    size_t framesToCopy = min(framesPerBuffer, decodedSamples->size() / CHANNELS);
    copy(decodedSamples->begin(), decodedSamples->begin() + framesToCopy * CHANNELS, output);

    // 使用済みサンプルを削除
    decodedSamples->erase(decodedSamples->begin(), decodedSamples->begin() + framesToCopy * CHANNELS);

    return paContinue;
}

int main()
{
    // Opusデコーダの初期化
    int error;
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK)
    {
        cerr << "Failed to create an Opus decoder: " << opus_strerror(error) << endl;
        return 1;
    }
    opus_decoder_ctl(decoder, OPUS_SET_GAIN(0)); // ゲインの設定

    // ファイルからエンコードされたデータを読み込む
    ifstream inputFile("output.opus", ios::binary);
    if (!inputFile.is_open())
    {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    vector<unsigned char> encodedData((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    // デコードされたサンプルを格納するベクタ
    vector<opus_int16> decodedSamples;

    // Opusデータをデコード
    size_t encodedOffset = 0;
    // Opusデータをデコード
    while (encodedOffset < encodedData.size())
    {
        opus_int16 outputFrame[FRAME_SIZE * CHANNELS];
        int frameSize = opus_decode(decoder, &encodedData[encodedOffset], encodedData.size() - encodedOffset, outputFrame, FRAME_SIZE, 0);
        if (frameSize < 0)
        {
            cerr << "Opus decoding failed: " << opus_strerror(frameSize) << endl;
            return 1;
        }
        decodedSamples.insert(decodedSamples.end(), outputFrame, outputFrame + frameSize * CHANNELS);
        encodedOffset += frameSize;
    }

    // PortAudioの初期化
    PaError paErr = Pa_Initialize();
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1;
    }

    // ストリームの開設
    PaStream *stream;
    paErr = Pa_OpenDefaultStream(&stream, 0, CHANNELS, paInt16, SAMPLE_RATE, FRAME_SIZE, playCallback, &decodedSamples);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1;
    }

    // ストリームの開始
    paErr = Pa_StartStream(stream);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1;
    }

    // 再生が完了するまで待つ
    while (Pa_IsStreamActive(stream) == 1)
    {
        Pa_Sleep(100);
    }

    // ストリームの停止
    paErr = Pa_StopStream(stream);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1;
    }

    // ストリームのクローズ
    paErr = Pa_CloseStream(stream);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1;
    }

    // PortAudioの終了
    Pa_Terminate();

    // Opusデコーダの解放
    opus_decoder_destroy(decoder);

    cout << "Playback finished." << endl;

    return 0;
}

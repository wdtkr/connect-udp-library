#include <iostream>
#include <vector>
#include <fstream>
#include <opus.h>
#include <portaudio.h>

#define SAMPLE_RATE 48000 // サンプリングレートを定義 (48kHz)
#define CHANNELS 1        // チャンネル数を定義 (モノラル)
#define FRAME_SIZE 960    // Opusフレームサイズを定義
#define MAX_FRAME_SIZE 6 * 960
#define BITRATE 64000 // ビットレートを定義 (64kbps)
#define APPLICATION OPUS_APPLICATION_AUDIO

using namespace std;

// レコーディングのコールバック関数
static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    vector<opus_int16> *recordedSamples = (vector<opus_int16> *)userData;
    const opus_int16 *input = (const opus_int16 *)inputBuffer;

    if (inputBuffer == NULL)
    {
        return paContinue; // 入力バッファが空の場合は続行
    }

    // 録音されたサンプルをベクタに追加
    recordedSamples->insert(recordedSamples->end(), input, input + framesPerBuffer * CHANNELS);
    return paContinue; // コールバックを続行
}

int main()
{
    PaError paErr;
    OpusEncoder *encoder;
    int error;

    // PortAudioの初期化
    paErr = Pa_Initialize();
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1; // 初期化エラー
    }

    vector<opus_int16> recordedSamples;

    // ストリームの開設
    PaStream *stream;
    paErr = Pa_OpenDefaultStream(&stream, CHANNELS, 0, paInt16, SAMPLE_RATE, FRAME_SIZE, recordCallback, &recordedSamples);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1; // ストリーム開設エラー
    }

    // ストリームの開始
    paErr = Pa_StartStream(stream);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1; // ストリーム開始エラー
    }

    // Opusエンコーダの初期化
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &error);
    if (error != OPUS_OK)
    {
        cerr << "Failed to create an Opus encoder: " << opus_strerror(error) << endl;
        return 1;
    }
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));                          // 複雑度の設定
    opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_SUPERWIDEBAND)); // 帯域幅の設定

    cout << "Recording. Press Enter to stop." << endl;
    getchar(); // Enterキーを押すまで録音

    // ストリームの停止
    paErr = Pa_StopStream(stream);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1; // ストリーム停止エラー
    }

    // ストリームのクローズ
    paErr = Pa_CloseStream(stream);
    if (paErr != paNoError)
    {
        cerr << "PortAudio error: " << Pa_GetErrorText(paErr) << endl;
        return 1; // ストリームクローズエラー
    }

    // PortAudioの終了
    Pa_Terminate();

    // エンコーディングとファイルへの書き込み
    vector<unsigned char> opusData(MAX_FRAME_SIZE);
    int numBytes;

    ofstream outputFile("output.opus", ios::binary);
    if (!outputFile.is_open())
    {
        cerr << "Failed to open output file." << endl;
        return 1;
    }

    // 録音データが十分に溜まるまで待機
    while (recordedSamples.size() < FRAME_SIZE * CHANNELS)
    {
        Pa_Sleep(100);
    }

    // Opusエンコードしてファイルに書き込む
    for (size_t i = 0; i + FRAME_SIZE * CHANNELS <= recordedSamples.size(); i += FRAME_SIZE * CHANNELS)
    {
        numBytes = opus_encode(encoder, &recordedSamples[i], FRAME_SIZE, opusData.data(), opusData.size());
        if (numBytes < 0)
        {
            cerr << "Opus encoding failed: " << opus_strerror(numBytes) << endl;
            return 1;
        }
        outputFile.write((const char *)opusData.data(), numBytes);
    }

    outputFile.close();
    opus_encoder_destroy(encoder);

    cout << "Recording finished and saved to 'output.opus'" << endl;

    return 0;
}

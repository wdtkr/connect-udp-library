#include <iostream>
#include <vector>
#include <mutex>
#include <dlfcn.h>
#include "mediaReceiver.hpp"
#include "opus/include/opus.h"
#include "codec/codec_api.h"
#include "uvgRTP/include/uvgrtp/lib.hh"

using namespace std;
using namespace uvgrtp;

// グローバル変数と定数
constexpr char LOCAL_ADDRESS[] = "127.0.0.1";
constexpr uint16_t VIDEO_LOCAL_PORT = 30002;
constexpr uint16_t AUDIO_LOCAL_PORT = 30004;

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960

void *handle;
ISVCDecoder *videoDecoder = nullptr;
OpusDecoder *audioDecoder = nullptr;

CallbackType debug_callback = nullptr;
ReceiveCallbackType receive_callback = nullptr;

int preDecodeVideoData(unsigned char *inputData, int inputLength, unsigned char *outputData, int outputLength)
{
    // OpenH264デコーダの初期化
    handle = dlopen("./libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return -1;
    }

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");

    if (createDecoder(&videoDecoder) != 0 || videoDecoder == nullptr)
    {
        cerr << "デコーダの作成に失敗" << endl;
        return -1;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if (videoDecoder->Initialize(&decParam) != 0)
    {
        cerr << "デコーダの初期化に失敗" << endl;
        return -1;
    }
}

// ビデオデータのデコードを行う関数
int decodeVideoData(unsigned char *inputData, int inputLength, unsigned char *outputData, int outputLength)
{

    SBufferInfo bufInfo;
    memset(&bufInfo, 0, sizeof(SBufferInfo));
    unsigned char *pData[3] = {nullptr, nullptr, nullptr};

    int ret = videoDecoder->DecodeFrameNoDelay(inputData, inputLength, pData, &bufInfo);
    if (ret == 0 && bufInfo.iBufferStatus == 1 && outputLength >= bufInfo.UsrData.sSystemBuffer.iWidth * bufInfo.UsrData.sSystemBuffer.iHeight * 3 / 2)
    {
        // YUVデータを出力バッファにコピー
        // ...
        // デコード成功
        return 0;
    }

    // デコード失敗
    return -1;
}

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

// オーディオデータのデコードを行う関数
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

void setCallback(CallbackType debug, ReceiveCallbackType receive)
{
    debug_callback = debug;
    receive_callback = receive;

    debug_callback("debugコールバック");
}

void test()
{
    // OpenH264デコーダの初期化
    handle = dlopen("./libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
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
        debug_callback("最後まで呼び出し成功");
    }
}

// // テスト用
int main()
{
    // OpenH264デコーダの初期化
    handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return -1;
    }

    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");

    if (createDecoder(&videoDecoder) != 0 || videoDecoder == nullptr)
    {
        cerr << "デコーダの作成に失敗" << endl;
        return -1;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if (videoDecoder->Initialize(&decParam) != 0)
    {
        cerr << "デコーダの初期化に失敗" << endl;
        return -1;
    }

    int opusErr;
    audioDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);
    if (opusErr != OPUS_OK)
    {
        cerr << "Opusデコーダの初期化に失敗" << endl;
        return -1;
    }

    cout << "処理終了" << endl;

    return 0;
}
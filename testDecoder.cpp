#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <opencv2/opencv.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_def.h"
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_app_def.h"

using namespace std;
using namespace cv;

int main()
{
    // ライブラリの読み込み
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return -1;
    }

    // デコーダの作成
    typedef int (*CreateDecoderFunc)(ISVCDecoder **);
    CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");
    ISVCDecoder *decoder = nullptr;
    if (createDecoder(&decoder) != 0 || decoder == nullptr)
    {
        cerr << "デコーダの作成に失敗" << endl;
        return -1;
    }

    // デコーダの初期化
    SDecodingParam decodingParam = {0};
    decodingParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
    if (decoder->Initialize(&decodingParam) != 0)
    {
        cerr << "デコーダの初期化に失敗" << endl;
        return -1;
    }

    // ファイルを開く
    ifstream videoFile("output.264", ios::in | ios::binary);
    if (!videoFile.is_open())
    {
        cerr << "ファイルを開けません" << endl;
        return -1;
    }

    // ファイルの内容を読み込む
    vector<unsigned char> videoData((istreambuf_iterator<char>(videoFile)), istreambuf_iterator<char>());
    videoFile.close();

    // デコード処理
    unsigned char *pData[3] = {nullptr, nullptr, nullptr};
    SBufferInfo bufInfo;
    memset(&bufInfo, 0, sizeof(SBufferInfo));

    int rv = decoder->DecodeFrame2(videoData.data(), videoData.size(), pData, &bufInfo);
    if (rv != 0)
    {
        cerr << "デコードに失敗" << endl;
        return -1;
    }

    // デコードされたフレームを表示
    if (bufInfo.iBufferStatus == 1)
    {
        Mat yuvImg(bufInfo.UsrData.sSystemBuffer.iHeight * 3 / 2, bufInfo.UsrData.sSystemBuffer.iWidth, CV_8UC1, pData[0]);
        Mat rgbImg;
        cvtColor(yuvImg, rgbImg, COLOR_YUV2BGR_I420);
        imshow("Decoded Frame", rgbImg);
        waitKey(0);
    }

    // デコーダの終了処理
    decoder->Uninitialize();
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(decoder);
    dlclose(handle);

    return 0;
}

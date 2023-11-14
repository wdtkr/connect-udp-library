#include <dlfcn.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/lib.hh"

// Tested with OpenCV 3.3
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

constexpr char REMOTE_ADDRESS[] = "127.0.0.1";
constexpr uint16_t REMOTE_PORT = 12345;

// the parameters of demostration
constexpr size_t PAYLOAD_LEN = 100;
constexpr int AMOUNT_OF_TEST_PACKETS = 100;
constexpr auto END_WAIT = std::chrono::seconds(5);

int main()
{
    uvgrtp::context ctx;

    // ライブラリの読み込み
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return -1;
    }

    // エンコーダの作成
    typedef int (*CreateEncoderFunc)(ISVCEncoder **);
    CreateEncoderFunc createEncoder = (CreateEncoderFunc)dlsym(handle, "WelsCreateSVCEncoder");
    ISVCEncoder *encoder = nullptr;
    if (createEncoder(&encoder) != 0 || encoder == nullptr)
    {
        cerr << "エンコーダの作成に失敗" << endl;
        return -1;
    }

    // エンコーダの初期化パラメータ
    SEncParamBase param;
    memset(&param, 0, sizeof(SEncParamBase));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME;
    param.fMaxFrameRate = 30;       // 一般的なフレームレート
    param.iPicWidth = 1920;         // 幅
    param.iPicHeight = 1080;        // 高さ
    param.iTargetBitrate = 5000000; // ビットレート
    if (encoder->Initialize(&param) != 0)
    {
        cerr << "エンコーダの初期化に失敗" << endl;
        return -1;
    }

    // カメラの初期化
    VideoCapture cap(1);
    if (!cap.isOpened())
    {
        cerr << "カメラが開けません" << endl;
        return -1;
    }

    Mat frame, yuv;
    int frameCount = 0;
    int captureDuration = 5; // 秒単位
    ofstream outFile("output.264", ios::out | ios::binary);

    while (frameCount < captureDuration * 30)
    {
        // 30FPSでエンコード
        cap >> frame;
        if (frame.empty())
            break;

        // YUVフォーマットに変換
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
            break;
        }

        // エンコードされたデータをファイルに書き込む
        for (int layer = 0; layer < info.iLayerNum; ++layer)
        {
            int layerSize = 0;
            SLayerBSInfo &layerInfo = info.sLayerInfo[layer];
            for (int nal = 0; nal < layerInfo.iNalCount; ++nal)
            {
                outFile.write(reinterpret_cast<char *>(layerInfo.pBsBuf + layerSize), layerInfo.pNalLengthInByte[nal]);
                layerSize += layerInfo.pNalLengthInByte[nal];
            }
        }
        frameCount++;
    }

    // エンコーダとファイルの解放
    encoder->Uninitialize();
    typedef void (*DestroyEncoderFunc)(ISVCEncoder *);
    DestroyEncoderFunc destroyEncoder = (DestroyEncoderFunc)dlsym(handle, "WelsDestroySVCEncoder");
    destroyEncoder(encoder);
    dlclose(handle);
    outFile.close();

    return 0;
}

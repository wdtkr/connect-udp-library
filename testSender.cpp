#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/lib.hh"

using namespace std;
using namespace cv;

constexpr char REMOTE_ADDRESS[] = "127.0.0.1";
constexpr uint16_t REMOTE_PORT = 30002;

// 1920x1080 ピクセルのフレームのための推定バッファサイズ、YUV 420 フォーマットの場合
size_t bufferSize = 1920 * 1080 * 3 / 2;

int main()
{
    // OpenCVの初期化
    VideoCapture cap(1);
    if (!cap.isOpened())
    {
        cerr << "カメラが開けません" << endl;
        return -1;
    }

    // OpenH264エンコーダの初期化
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return -1;
    }

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
    param.fMaxFrameRate = 30;
    // param.iPicWidth = 1920;
    // param.iPicHeight = 1080;
    param.iPicWidth = 640;
    param.iPicHeight = 480;
    param.iTargetBitrate = 5000000;
    if (encoder->Initialize(&param) != 0)
    {
        cerr << "エンコーダの初期化に失敗" << endl;
        return -1;
    }

    int videoFormat = videoFormatI420;
    encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

    // uvgRTPの初期化
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(REMOTE_ADDRESS);
    int send_flags = RCE_FRAGMENT_GENERIC | RCE_SEND_ONLY;
    uvgrtp::media_stream *strm = sess->create_stream(REMOTE_PORT, RTP_FORMAT_H264, send_flags);
    strm->configure_ctx(RCC_MTU_SIZE, 1500);

    // test
    ofstream outFile("output.264", ios::out | ios::binary);
    bool flag = true;
    std::thread t([&flag]()
                  {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        flag = false;
        std::cout << "フラグの状態を変更しました: " << flag << std::endl; });

    Mat frame, yuv;
    while (flag)
    {
        cap >> frame;
        if (frame.empty())
            break;

        cvtColor(frame, yuv, COLOR_BGR2YUV_I420);

        // cout << "エンコード前：" << frame << endl;

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

        // cout << "レイヤー数: " << info.iLayerNum << endl;

        // エンコードされたデータをRTPで送信
        for (int layer = 0; layer < info.iLayerNum; ++layer)
        {
            int layerSize = 0;
            SLayerBSInfo &layerInfo = info.sLayerInfo[layer];
            // cout << "レイヤー " << layer << "、NALユニット数: " << layerInfo.iNalCount << endl;

            for (int nal = 0; nal < layerInfo.iNalCount; ++nal)
            {
                if (layerInfo.pBsBuf == nullptr)
                {
                    cerr << "エンコードされたデータが nullptr です。" << endl;
                    // エラー処理
                }

                // cout << "エンコード後!：" << reinterpret_cast<char *>(layerInfo.pBsBuf + layerSize) << endl;
                // cout << "NALユニットのサイズ: " << layerInfo.pNalLengthInByte[nal] << endl;

                if (layerInfo.pNalLengthInByte[nal] + layerSize > bufferSize)
                { // bufferSize はエンコーディングバッファのサイズ
                    cerr << "NALユニットのサイズがバッファサイズを超えています。" << endl;
                    // エラー処理
                }

                // cout << "送信データのアドレス: " << static_cast<void *>(layerInfo.pBsBuf + layerSize) << endl;

                if (strm->push_frame(layerInfo.pBsBuf + layerSize, layerInfo.pNalLengthInByte[nal], RCE_SEND_ONLY) != RTP_OK)
                {
                    cerr << "RTPフレームの送信に失敗しました" << endl;
                }
                // test
                outFile.write(reinterpret_cast<char *>(layerInfo.pBsBuf + layerSize), layerInfo.pNalLengthInByte[nal]);
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

    // test
    outFile.close();

    return 0;
}
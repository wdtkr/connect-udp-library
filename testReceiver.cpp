#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/lib.hh"

using namespace std;
using namespace cv;
using namespace uvgrtp;

constexpr char LOCAL_ADDRESS[] = "127.0.0.1";
constexpr uint16_t LOCAL_PORT = 30002;

int main()
{
    // uvgRTPのコンテキストとセッションを作成
    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(LOCAL_ADDRESS);

    // RTPストリームを作成（受信専用）
    int flags = RCE_RECEIVE_ONLY | RCE_FRAGMENT_GENERIC;
    uvgrtp::media_stream *h264 = sess->create_stream(LOCAL_PORT, RTP_FORMAT_H264, flags);

    // OpenH264デコーダの初期化
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return -1;
    }

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
    if (decoder->Initialize(&decParam) != 0)
    {
        cerr << "デコーダの初期化に失敗" << endl;
        return -1;
    }

    namedWindow("Decoded Frame", WINDOW_AUTOSIZE);

    while (true)
    {
        // uvgRTPからデータを受信
        frame::rtp_frame *frame = h264->pull_frame();
        if (!frame)
        {
            cerr << "RTPフレームの受信に失敗しました" << endl;
            continue;
        }
        else
        {
            cout << "RTPフレームの受信に成功しました" << endl;
        }

        // デコード処理
        SBufferInfo bufInfo;
        memset(&bufInfo, 0, sizeof(SBufferInfo));
        unsigned char *pData[3] = {nullptr, nullptr, nullptr};

        int ret = decoder->DecodeFrameNoDelay(frame->payload, frame->payload_len, pData, &bufInfo);
        // uvgrtp::frame::dealloc_frame(frame);

        if (ret == 0 && bufInfo.iBufferStatus == 1)
        {
            Mat yuvImg(bufInfo.UsrData.sSystemBuffer.iHeight * 3 / 2, bufInfo.UsrData.sSystemBuffer.iWidth, CV_8UC1, pData);
            Mat rgbImg;
            cvtColor(yuvImg, rgbImg, COLOR_YUV2BGR_I420);
            imshow("Decoded Frame", rgbImg);
            if (waitKey(30) >= 0)
                break; // キー入力で終了
        }
        else
        {
            cout << "デコード失敗" << endl;
        }
    }

    // デコーダの解放
    decoder->Uninitialize();
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(decoder);
    dlclose(handle);

    // uvgRTPセッションの解放
    if (h264)
    {
        sess->destroy_stream(h264);
    }
    if (sess)
    {
        ctx.destroy_session(sess);
    }

    return 0;
}

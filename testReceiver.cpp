#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <fstream>
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
    int receive_flags = RCE_RECEIVE_ONLY | RCE_NO_H26X_PREPEND_SC;
    uvgrtp::media_stream *h264 = sess->create_stream(LOCAL_PORT, RTP_FORMAT_H264, receive_flags);
    h264->configure_ctx(RCC_RING_BUFFER_SIZE, 40000000);

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
        frame::rtp_frame *frame = h264->pull_frame(1000);
        if (!frame)
        {
            cerr << "RTPフレームの受信に失敗しました" << endl;
            continue;
        }
        // else
        // {
        //     cout << "フレームの受信に成功" << &frame << endl;
        // }

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
            cv::Mat yMat(height, width, CV_8UC1, pData[0], yStride);

            // U成分とV成分のMatオブジェクトを作成
            cv::Mat uMat(height / 2, width / 2, CV_8UC1, pData[1], uvStride);
            cv::Mat vMat(height / 2, width / 2, CV_8UC1, pData[2], uvStride);

            // YUV420p形式に合わせてY, U, Vデータを配置
            cv::Mat yuvImg(height + height / 2, width, CV_8UC1);
            yMat.copyTo(yuvImg(cv::Rect(0, 0, width, height)));

            // U成分とV成分の配置を修正
            // U成分を配置
            cv::Mat uPart(uMat.size(), CV_8UC1, yuvImg.data + height * width);
            uMat.copyTo(uPart);

            // V成分を配置
            cv::Mat vPart(vMat.size(), CV_8UC1, yuvImg.data + height * width + (height / 2) * (width / 2));
            vMat.copyTo(vPart);

            // YUVからRGBへ変換
            cv::Mat rgbImg;
            cv::cvtColor(yuvImg, rgbImg, cv::COLOR_YUV2BGR_I420);

            imshow("Decoded Frame", rgbImg);
        }

        // ウィンドウを更新
        int key = waitKey(1);
        if (key == 'q' || key == 'Q') // 'q' または 'Q' が押された場合、ループを抜ける
        {
            break;
        }

        // RTPフレームのメモリを解放
        uvgrtp::frame::dealloc_frame(frame);
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

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
constexpr uint16_t LOCAL_PORT = 12345;

// int main()
// {
//     // uvgRTPのコンテキストとセッションを作成
//     uvgrtp::context ctx;
//     uvgrtp::session *sess = ctx.create_session(LOCAL_ADDRESS);

//     // RTPストリームを作成（受信専用）
//     int flags = RCE_RECEIVE_ONLY;
//     uvgrtp::media_stream *h264 = sess->create_stream(LOCAL_PORT, RTP_FORMAT_H264, flags);

//     // ライブラリの読み込み
//     void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
//     if (!handle)
//     {
//         cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
//         return -1;
//     }

//     // デコーダの作成
//     typedef int (*CreateDecoderFunc)(ISVCDecoder **);
//     CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");
//     ISVCDecoder *decoder = nullptr;
//     if (createDecoder(&decoder) != 0 || decoder == nullptr)
//     {
//         cerr << "デコーダの作成に失敗" << endl;
//         return -1;
//     }

//     // デコーダの初期化
//     SDecodingParam sDecParam = {0};
//     sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
//     decoder->Initialize(&sDecParam);

//     while (true)
//     {
//         // uvgRTPからデータを受信
//         frame::rtp_frame *frame = h264->pull_frame();
//         if (!frame)
//         {
//             cerr << "RTPフレームの受信に失敗しました" << endl;
//             continue;
//         }

//         // デコード処理
//         int iRet = 0;
//         unsigned char *pData[3] = {nullptr, nullptr, nullptr}; // Y, U, Vバッファ
//         SBufferInfo sDstBufInfo;
//         memset(&sDstBufInfo, 0, sizeof(SBufferInfo));

//         iRet = decoder->DecodeFrameNoDelay(frame->payload, frame->payload_len, pData, &sDstBufInfo);
//         frame::dealloc_frame(frame); // フレームメモリの解放

//         // デコードに成功した場合、OpenCVを用いて画像を表示
//         if (iRet == 0 && sDstBufInfo.iBufferStatus == 1)
//         {
//             Mat yuvImg(sDstBufInfo.UsrData.sSystemBuffer.iHeight * 3 / 2, sDstBufInfo.UsrData.sSystemBuffer.iWidth, CV_8UC1, pData[0]);
//             Mat rgbImg;
//             cvtColor(yuvImg, rgbImg, COLOR_YUV2BGR_I420);
//             imshow("Decoded Frame", rgbImg);
//             if (waitKey(30) >= 0)
//                 break; // 30msごとにフレームを更新し、キー入力があれば終了
//         }
//     }

//     // デコーダの解放
//     decoder->Uninitialize();
//     typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
//     DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
//     destroyDecoder(decoder);
//     dlclose(handle);

//     // uvgRTPセッションの解放
//     if (h264)
//     {
//         sess->destroy_stream(h264);
//     }
//     if (sess)
//     {
//         ctx.destroy_session(sess);
//     }

//     return 0;
// }

constexpr auto RECEIVE_TIME_MS = std::chrono::milliseconds(10000);
constexpr int RECEIVER_WAIT_TIME_MS = 100;

void process_frame(uvgrtp::frame::rtp_frame *frame);

int main(void)
{
    std::cout << "Starting uvgRTP RTP receive hook example" << std::endl;

    uvgrtp::context ctx;
    uvgrtp::session *sess = ctx.create_session(LOCAL_ADDRESS);
    int flags = RCE_RECEIVE_ONLY;

    uvgrtp::media_stream *receiver = sess->create_stream(LOCAL_PORT, RTP_FORMAT_H265, flags);

    if (receiver)
    {
        std::cout << "Start receiving frames for " << RECEIVE_TIME_MS.count() << " ms" << std::endl;
        auto start = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() - start < RECEIVE_TIME_MS)
        {
            /* You can specify a timeout for the operation and if the a frame is not received
             * within that time limit, pull_frame() returns a nullptr
             *
             * The parameter tells how long time a frame is waited in milliseconds */
            uvgrtp::frame::rtp_frame *frame = receiver->pull_frame(RECEIVER_WAIT_TIME_MS);

            if (frame)
                process_frame(frame);
        }

        sess->destroy_stream(receiver);
    }

    if (sess)
    {
        /* Session must be destroyed manually */
        ctx.destroy_session(sess);
    }

    return EXIT_SUCCESS;
}

void process_frame(uvgrtp::frame::rtp_frame *frame)
{
    std::cout << "Received an RTP frame" << std::endl;

    /* When we receive a frame, the ownership of the frame belongs to us and
     * when we're done with it, we need to deallocate the frame */
    (void)uvgrtp::frame::dealloc_frame(frame);
}

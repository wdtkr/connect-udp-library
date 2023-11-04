#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

int main()
{
    // カメラから映像をキャプチャ
    cv::VideoCapture capture(0);

    if (!capture.isOpened())
    {
        std::cerr << "カメラが利用できません" << std::endl;
        return -1;
    }

    // FFmpegの初期化
    avformat_network_init();

    // 出力ファイルの設定（RTPへの送信の場合はstdoutを使用）
    AVFormatContext *formatContext = nullptr; // formatContextを追加
    avformat_alloc_output_context2(&formatContext, NULL, "rtp", "rtp://localhost:1234");
    if (!formatContext)
    {
        std::cerr << "出力フォーマットの設定に失敗しました" << std::endl;
        return -1;
    }

    // ビデオストリームの設定
    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codec || !codecContext)
    {
        std::cerr << "エンコーダの初期化に失敗しました" << std::endl;
        return -1;
    }

    AVStream *videoStream = avformat_new_stream(formatContext, codec);
    if (!videoStream)
    {
        std::cerr << "ビデオストリームの設定に失敗しました" << std::endl;
        return -1;
    }

    // FFmpegのエンコーダコンテキストの初期化
    if (avcodec_open2(codecContext, codec, NULL) < 0)
    {
        std::cerr << "エンコーダコンテキストの初期化に失敗しました" << std::endl;
        return -1;
    }

    // ビデオストリームの設定
    codecContext->width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    codecContext->height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext->time_base = {1, static_cast<int>(capture.get(cv::CAP_PROP_FPS))};

    // エンコードとストリーミングのループ
    cv::Mat frame;
    AVPacket *packet = av_packet_alloc(); // パケットを追加
    if (!packet)
    {
        std::cerr << "パケットの初期化に失敗しました" << std::endl;
        return -1;
    }

    SwsContext *swsContext = sws_getContext(codecContext->width, codecContext->height,
                                            AV_PIX_FMT_BGR24,
                                            codecContext->width, codecContext->height,
                                            codecContext->pix_fmt,
                                            SWS_BILINEAR, NULL, NULL, NULL);

    while (capture.read(frame))
    {
        // カメラからのフレームをFFmpeg用のフレームに変換
        AVFrame *inputFrame = av_frame_alloc();
        if (!inputFrame)
        {
            std::cerr << "フレームの初期化に失敗しました" << std::endl;
            break;
        }

        inputFrame->width = codecContext->width;
        inputFrame->height = codecContext->height;
        inputFrame->format = AV_PIX_FMT_BGR24;

        av_frame_get_buffer(inputFrame, 0);

        // 画像データのポインタの配列を作成
        const uint8_t *srcSlices[] = {frame.ptr()};

        // 画像データの各行のサイズの配列を作成
        const int srcStride[] = {static_cast<int>(frame.step)};

        // sws_scale 関数を呼び出し
        sws_scale(swsContext, srcSlices, srcStride, 0, frame.rows, inputFrame->data, inputFrame->linesize);

        // フレームのエンコード
        if (avcodec_send_frame(codecContext, inputFrame) < 0)
        {
            std::cerr << "フレームの送信に失敗しました" << std::endl;
            av_frame_free(&inputFrame);
            break;
        }

        // エンコードされたデータをパケットに書き込む
        if (avcodec_receive_packet(codecContext, packet) < 0)
        {
            std::cerr << "パケットの受信に失敗しました" << std::endl;
            av_frame_free(&inputFrame);
            break;
        }

        // フレームをRTPで送信
        av_packet_rescale_ts(packet, codecContext->time_base, videoStream->time_base);
        av_interleaved_write_frame(formatContext, packet);

        // フレームとパケットの後始末
        av_frame_free(&inputFrame);
        av_packet_unref(packet);

        // キー入力をチェックして終了条件を設定
        if (cv::waitKey(1) == 27)
            break;
    }

    // 後始末
    av_write_trailer(formatContext);
    avcodec_free_context(&codecContext);
    avformat_free_context(formatContext);
    av_packet_free(&packet); // パケットの解放を追加

    return 0;
}

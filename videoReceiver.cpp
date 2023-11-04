#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

int main()
{
    AVFormatContext *formatContext = avformat_alloc_context();
    avformat_open_input(&formatContext, "rtp://localhost:1234", NULL, NULL);
    avformat_find_stream_info(formatContext, NULL);

    int videoStreamIndex = -1;

    for (int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }

    // デコーダの初期化
    const AVCodec *decoder = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
    AVCodecContext *decoderContext = avcodec_alloc_context3(decoder);
    avcodec_open2(decoderContext, decoder, NULL);

    // 画像表示の初期化
    cv::namedWindow("Received Video", cv::WINDOW_NORMAL);
    cv::Mat frame;

    AVPacket packet;
    av_init_packet(&packet);

    // デコードと表示のループ
    while (av_read_frame(formatContext, &packet) >= 0)
    {
        if (packet.stream_index == videoStreamIndex)
        {
            // パケットのデコード
            // ここでデコード処理を行う

            // デコードしたフレームをOpenCVのMatにコピー
            // ここでフレームをMatにコピー

            // 画像を表示
            cv::imshow("Received Video", frame);

            // キー入力をチェックして終了条件を設定
            if (cv::waitKey(1) == 27)
                break;
        }

        av_packet_unref(&packet);
    }

    // 後始末
    cv::destroyWindow("Received Video");
    avcodec_free_context(&decoderContext);
    avformat_close_input(&formatContext);

    return 0;
}

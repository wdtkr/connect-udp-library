#include "openh264-2.3.1/codec/api/wels/codec_api.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

bool EncodeFrame(ISVCEncoder *encoder, const SSourcePicture &srcPic, SFrameBSInfo &info)
{
    int rv = encoder->EncodeFrame(&srcPic, &info);
    if (rv != cmResultSuccess)
    {
        std::cerr << "Failed to encode frame" << std::endl;
        return false;
    }
    return true;
}

int CameraTest()
{
    cv::VideoCapture cap(1); // デバイスのオープン
    // cap.open(0);//こっちでも良い．

    if (!cap.isOpened()) // カメラデバイスが正常にオープンしたか確認．
    {
        // 読み込みに失敗したときの処理
        return -1;
    }

    cv::Mat frame;          // 取得したフレーム
    while (cap.read(frame)) // 無限ループ
    {
        //
        // 取得したフレーム画像に対して，クレースケール変換や2値化などの処理を書き込む．
        //

        cv::imshow("win", frame); // 画像を表示．
        const int key = cv::waitKey(1);
        if (key == 'q' /*113*/) // qボタンが押されたとき
        {
            break; // whileループから抜ける．
        }
        else if (key == 's' /*115*/) // sが押されたとき
        {
            // フレーム画像を保存する．
            cv::imwrite("img.png", frame);
        }
    }
    cv::destroyAllWindows();
    return 0;
}

int VideoEncode()
{
    // カメラを起動する
    cv::VideoCapture capture(1); // カメラデバイスのIDは環境によって異なる場合があります
    if (!capture.isOpened())
    {
        std::cerr << "Error opening camera" << std::endl;
        return -1;
    }

    // エンコーダの設定
    ISVCEncoder *encoder = nullptr;
    int rv = WelsCreateSVCEncoder(&encoder);
    if (rv != 0 || encoder == nullptr)
    {
        std::cerr << "Failed to create encoder" << std::endl;
        return -1;
    }

    // エンコーダのパラメータを設定する
    SEncParamBase param;
    memset(&param, 0, sizeof(SEncParamBase));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME;
    param.fMaxFrameRate = capture.get(cv::CAP_PROP_FPS); // カメラのフレームレートを取得
    param.iPicWidth = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    param.iPicHeight = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    param.iTargetBitrate = 5000000; // 5 Mbps

    encoder->Initialize(&param);

    // ビットストリーム情報を初期化する
    SFrameBSInfo info;
    memset(&info, 0, sizeof(SFrameBSInfo));

    // フレームのピクチャ情報を初期化する
    SSourcePicture pic;
    memset(&pic, 0, sizeof(SSourcePicture));
    pic.iPicWidth = param.iPicWidth;
    pic.iPicHeight = param.iPicHeight;
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = pic.iPicWidth;
    pic.iStride[1] = pic.iPicWidth / 2;
    pic.iStride[2] = pic.iPicWidth / 2;

    // ファイルストリームを開く
    std::ofstream outFile("output.264", std::ios::binary | std::ios::out);

    // 指定された時間だけ映像を取得する
    cv::Mat frame;
    int frameCount = param.fMaxFrameRate * 5; // 5秒間のフレーム数
    while (frameCount-- > 0)
    {
        capture >> frame;
        if (frame.empty())
        {
            break;
        }

        cv::Mat yuv;
        cv::cvtColor(frame, yuv, cv::COLOR_BGR2YUV_I420);

        // YUVフレームデータをpicに設定する
        pic.pData[0] = yuv.data;
        pic.pData[1] = yuv.data + pic.iPicWidth * pic.iPicHeight;
        pic.pData[2] = yuv.data + pic.iPicWidth * pic.iPicHeight * 5 / 4;

        if (!EncodeFrame(encoder, pic, info))
        {
            break;
        }

        // エンコードされたデータをファイルに書き出す
        for (int i = 0; i < info.iLayerNum; ++i)
        {
            const SLayerBSInfo &layerInfo = info.sLayerInfo[i];
            for (int j = 0; j < layerInfo.iNalCount; ++j)
            {
                outFile.write(reinterpret_cast<char *>(layerInfo.pBsBuf + layerInfo.pNalLengthInByte[j]), layerInfo.pNalLengthInByte[j]);
            }
        }
    }

    // リソースのクリーンアップ
    encoder->Uninitialize();
    WelsDestroySVCEncoder(encoder);
    outFile.close();

    return 0;
}

int VideoDecode()
{
    // OpenH264 ライブラリを初期化
    if (WelsInit() != 0)
    {
        std::cerr << "Failed to initialize OpenH264" << std::endl;
        return -1;
    }

    // デコーダの設定
    ISVCDecoder *decoder = nullptr;
    if (WelsCreateDecoder(&decoder) != 0 || decoder == nullptr)
    {
        std::cerr << "Failed to create decoder" << std::endl;
        return -1;
    }

    SDecodingParam decParam;
    memset(&decParam, 0, sizeof(SDecodingParam));

    // ビデオファイルを開く
    const char *videoFileName = "your_h264_file.264"; // ファイル名を適切に設定
    FILE *videoFile = fopen(videoFileName, "rb");
    if (videoFile == nullptr)
    {
        std::cerr << "Failed to open video file" << std::endl;
        return -1;
    }

    // フレームの表示用ウィンドウを作成
    cv::namedWindow("Decoded Video", cv::WINDOW_NORMAL);

    SFrameBSInfo frameInfo;
    memset(&frameInfo, 0, sizeof(SFrameBSInfo));

    while (true)
    {
        int bytesRead = fread(frameInfo.sLayerInfo[0].pBsBuf, 1, MAX_DATA_SIZE, videoFile);
        if (bytesRead <= 0)
        {
            // ビデオファイルの終了
            break;
        }

        // ビデオフレームをデコード
        if (decoder->DecodeFrame2(frameInfo.sLayerInfo[0].pBsBuf, bytesRead, NULL, &decParam) != 0)
        {
            std::cerr << "Failed to decode frame" << std::endl;
            break;
        }

        // デコードされたフレームを取得
        SBufferInfo bufferInfo;
        if (decoder->GetOption(DECODER_OPTION_GET_OUTPUT, &bufferInfo) != 0)
        {
            std::cerr << "Failed to get decoded frame" << std::endl;
            break;
        }

        if (bufferInfo.iBufferStatus == 1)
        {
            cv::Mat frame(bufferInfo.UsrData.sSystemBuffer.iHeight, bufferInfo.UsrData.sSystemBuffer.iWidth, CV_8UC3, bufferInfo.UsrData.sSystemBuffer.pBuffer[0]);
            cv::imshow("Decoded Video", frame);
            cv::waitKey(1);
        }
    }

    // リソースのクリーンアップ
    WelsDestroyDecoder(decoder);
    fclose(videoFile);
    WelsCleanup();

    return 0;
}

int main()
{
    // CameraTest();
    VideoEncode();

    return 0;
}

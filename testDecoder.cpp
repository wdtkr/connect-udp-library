#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"

using namespace std;
using namespace cv;

int test = 0;

// NALユニットの開始コードを検出する関数
vector<size_t> FindNALUnits(const vector<uint8_t> &buffer)
{
    vector<size_t> nalStarts;
    for (size_t i = 0; i < buffer.size() - 4; ++i)
    {
        if (buffer[i] == 0x00 && buffer[i + 1] == 0x00 && buffer[i + 2] == 0x00 && buffer[i + 3] == 0x01)
        {
            nalStarts.push_back(i);
        }
    }
    return nalStarts;
}

vector<uint8_t> readFile(const string &filename)
{
    ifstream file(filename, ios::binary);
    return vector<uint8_t>(istreambuf_iterator<char>(file), {});
}

void logNALUnitTypes(const vector<uint8_t> &buffer)
{
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        if (i + 4 < buffer.size() &&
            buffer[i] == 0x00 && buffer[i + 1] == 0x00 &&
            buffer[i + 2] == 0x00 && buffer[i + 3] == 0x01)
        {
            uint8_t nal_unit_type = buffer[i + 4] & 0x1F;
            cout << "NAL Unit Type: " << static_cast<int>(nal_unit_type) << endl;
            i += 4; // スキップして次のNALユニットを探します
        }
    }
}

int main()
{
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
        dlclose(handle);
        return -1;
    }

    SDecodingParam decParam = {0};
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    decoder->Initialize(&decParam);

    // ファイルを開く
    ifstream videoFile("receive.264", ios::in | ios::binary);
    if (!videoFile.is_open())
    {
        cerr << "ファイルを開けません" << endl;
        return -1;
    }

    // ファイルの内容を読み込む
    vector<uint8_t> buffer((istreambuf_iterator<char>(videoFile)), istreambuf_iterator<char>());
    videoFile.close();

    logNALUnitTypes(buffer);

    // NALユニットの境界を見つける
    vector<size_t> nalStarts = FindNALUnits(buffer);

    cout << "nalStarts.size: " << nalStarts.size() << endl;

    namedWindow("Decoded Frame", WINDOW_AUTOSIZE);

    int a = 0;

    // 各NALユニットをデコード
    for (size_t i = 0; i < nalStarts.size(); ++i)
    {
        size_t end = (i < nalStarts.size() - 1) ? nalStarts[i + 1] : buffer.size();
        size_t size = end - nalStarts[i];

        // デコード処理
        SBufferInfo bufInfo;
        memset(&bufInfo, 0, sizeof(SBufferInfo));
        unsigned char *pData[3] = {nullptr, nullptr, nullptr};

        cout << "&buffer[nalStarts[i]] : " << buffer[nalStarts[i]] << ",,, size : " << size << ",,, pData : " << pData << " ,,, bufInfo : " << &bufInfo << endl;

        // 受信したペイロードの最初の数バイトをログ出力（デバッグ用）
        // int min_len = std::min(static_cast<size_t>(10), size);
        // cout << "First bytes of payload: ";
        // for (size_t j = 0; j < min_len; ++j)
        //     cout << hex << (int)buffer[nalStarts[i] + j] << " ";
        // cout << dec << endl;

        if (test == 10)
        {
            cout << "First bytes of payload: ";
            for (size_t j = 0; j < size; ++j)
                cout << hex << (int)buffer[nalStarts[i] + j] << " ";
            cout << dec << endl;
        }

        test += 1;

        int ret = decoder->DecodeFrameNoDelay(&buffer[nalStarts[i]], size, pData, &bufInfo);
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
            waitKey(1000 / 33);
        }
        // else
        // {
        //     cerr << "エラーコード ret： " << ret << " ,,, bufInfo.iBufferStatus： " << bufInfo.iBufferStatus << endl;
        // }
    }

    // デコーダの終了処理
    decoder->Uninitialize();
    typedef void (*DestroyDecoderFunc)(ISVCDecoder *);
    DestroyDecoderFunc destroyDecoder = (DestroyDecoderFunc)dlsym(handle, "WelsDestroyDecoder");
    destroyDecoder(decoder);
    dlclose(handle);

    return 0;
}

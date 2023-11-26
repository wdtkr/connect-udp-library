#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <opus.h>
#include "mediaSender.hpp"
#include "/Users/takeru/Downloads/openh264-2.3.1/codec/api/wels/codec_api.h"
#include "/Users/takeru/Documents/GitHub/connect-udp-library/uvgRTP/include/uvgrtp/lib.hh"

using namespace std;
using namespace uvgrtp;

// 映像関連の定数
constexpr char REMOTE_ADDRESS[] = "127.0.0.1";
constexpr uint16_t VIDEO_REMOTE_PORT = 30002;
const int width = 1920;
const int height = 1080;
const int frameRate = 30;
const int bitrate = 5000000;
const int mtuSize = 1400;
const int send_flags = RCE_PACE_FRAGMENT_SENDING | RCE_SEND_ONLY;

// オーディオ関連の定数
constexpr uint16_t AUDIO_REMOTE_PORT = 30004;
constexpr int SAMPLE_RATE = 48000;
constexpr int CHANNELS = 1;
constexpr int FRAME_SIZE = 960;
constexpr int MAX_PACKET_SIZE = 1500;

ISVCEncoder *videoEncoder = nullptr;
media_stream *videoStrm = nullptr;

OpusEncoder *audioEncoder = nullptr;
media_stream *audioStrm = nullptr;

// ビデオデータ送信準備処理
void preSendVideoData(unsigned char *data, int dataLength)
{
    // OpenH264エンコーダの初期化
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        cerr << "ライブラリの読み込みに失敗: " << dlerror() << endl;
        return;
    }

    typedef int (*CreateEncoderFunc)(ISVCEncoder **);
    CreateEncoderFunc createEncoder = (CreateEncoderFunc)dlsym(handle, "WelsCreateSVCEncoder");
    if (createEncoder(&videoEncoder) != 0 || videoEncoder == nullptr)
    {
        cerr << "エンコーダの作成に失敗" << endl;
        return;
    }

    SEncParamBase param;
    memset(&param, 0, sizeof(SEncParamBase));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME;
    param.fMaxFrameRate = frameRate;
    param.iPicWidth = width;
    param.iPicHeight = height;
    param.iTargetBitrate = bitrate;
    if (videoEncoder->Initialize(&param) != 0)
    {
        cerr << "エンコーダの初期化に失敗" << endl;
        return;
    }

    int videoFormat = videoFormatI420;
    videoEncoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

    // uvgRTPの初期化
    context ctx;
    session *sess = ctx.create_session(REMOTE_ADDRESS);

    // ここでエラー出るかも
    videoStrm = sess->create_stream(VIDEO_REMOTE_PORT, RTP_FORMAT_H264, send_flags);
    videoStrm->configure_ctx(RCC_MTU_SIZE, mtuSize);

    while (true)
    {

        // エンコード
        SSourcePicture pic;
        memset(&pic, 0, sizeof(SSourcePicture));
        pic.iPicWidth = width;
        pic.iPicHeight = height;
        pic.iColorFormat = videoFormatI420;
        pic.iStride[0] = width;
        pic.iStride[1] = pic.iStride[2] = width / 2;
        pic.pData[0] = data; // YUVデータをそのまま使用

        SFrameBSInfo info;
        memset(&info, 0, sizeof(SFrameBSInfo));
        if (videoEncoder->EncodeFrame(&pic, &info) != 0)
        {
            cerr << "エンコードに失敗" << endl;
            continue;
        }

        // エンコードされたデータをRTPで送信
        for (int layer = 0; layer < info.iLayerNum; ++layer)
        {
            int layerSize = 0;
            SLayerBSInfo &layerInfo = info.sLayerInfo[layer];

            for (int nal = 0; nal < layerInfo.iNalCount; ++nal)
            {

                if (videoStrm->push_frame(layerInfo.pBsBuf + layerSize, layerInfo.pNalLengthInByte[nal], RTP_NO_H26X_SCL) != RTP_OK)
                {
                    cerr << "RTPフレームの送信に失敗しました" << endl;
                }

                layerSize += layerInfo.pNalLengthInByte[nal];
            }
        }
    }

    // クリーンアップ
    videoEncoder->Uninitialize();
    typedef void (*DestroyEncoderFunc)(ISVCEncoder *);
    DestroyEncoderFunc destroyEncoder = (DestroyEncoderFunc)dlsym(handle, "WelsDestroySVCEncoder");
    destroyEncoder(videoEncoder);
    dlclose(handle);
    sess->destroy_stream(videoStrm);
    ctx.destroy_session(sess);
}

void sendVideoData(unsigned char *data, int dataLength)
{
    // while (true)
    // {
    // エンコード
    SSourcePicture pic;
    memset(&pic, 0, sizeof(SSourcePicture));
    pic.iPicWidth = width;
    pic.iPicHeight = height;
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = width;
    pic.iStride[1] = pic.iStride[2] = width / 2;
    pic.pData[0] = data; // YUVデータをそのまま使用

    SFrameBSInfo info;
    memset(&info, 0, sizeof(SFrameBSInfo));
    if (videoEncoder->EncodeFrame(&pic, &info) != 0)
    {
        cerr << "エンコードに失敗" << endl;
        return;
    }

    // エンコードされたデータをRTPで送信
    for (int layer = 0; layer < info.iLayerNum; ++layer)
    {
        int layerSize = 0;
        SLayerBSInfo &layerInfo = info.sLayerInfo[layer];

        for (int nal = 0; nal < layerInfo.iNalCount; ++nal)
        {

            if (videoStrm->push_frame(layerInfo.pBsBuf + layerSize, layerInfo.pNalLengthInByte[nal], RTP_NO_H26X_SCL) != RTP_OK)
            {
                cerr << "RTPフレームの送信に失敗しました" << endl;
            }

            layerSize += layerInfo.pNalLengthInByte[nal];
        }
    }
    // }
}

void preSendAudioData(unsigned char *data, int dataLength)
{
    // Opusエンコーダーの初期化
    int opusErr;
    audioEncoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusErr);

    // uvgRTPの初期化
    context ctx;
    session *sess = ctx.create_session(REMOTE_ADDRESS);
    audioStrm = sess->create_stream(AUDIO_REMOTE_PORT, RTP_FORMAT_OPUS, RCE_SEND_ONLY);
    audioStrm->configure_ctx(RCC_MTU_SIZE, 1400);

    // 生データをエンコード
    vector<unsigned char> opusData(MAX_PACKET_SIZE);
    int numBytes = opus_encode(audioEncoder, reinterpret_cast<opus_int16 *>(data), FRAME_SIZE, opusData.data(), opusData.size());
    if (numBytes < 0)
    {
        cerr << "Opus encoding failed" << endl;
    }
    else
    {
        // エンコードされたデータをRTPで送信
        audioStrm->push_frame(opusData.data(), numBytes, RTP_NO_FLAGS);
    }

    // クリーンアップ
    opus_encoder_destroy(audioEncoder);
    sess->destroy_stream(audioStrm);
    ctx.destroy_session(sess);
}

void sendAudioData(unsigned char *data, int dataLength)
{

    // 生データをエンコード
    vector<unsigned char> opusData(MAX_PACKET_SIZE);
    int numBytes = opus_encode(audioEncoder, reinterpret_cast<opus_int16 *>(data), FRAME_SIZE, opusData.data(), opusData.size());
    if (numBytes < 0)
    {
        cerr << "Opus encoding failed" << endl;
    }
    else
    {
        // エンコードされたデータをRTPで送信
        audioStrm->push_frame(opusData.data(), numBytes, RTP_NO_FLAGS);
    }
}
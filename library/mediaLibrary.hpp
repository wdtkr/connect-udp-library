#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

typedef void (*ReceiveCallbackType)(unsigned char *, int, int);
typedef void (*CallbackType)(const char *);

extern "C"
{
    UNITYEXPORT void UNITYCALLCONV setMediaCallback(CallbackType debug, ReceiveCallbackType receiveVideo, ReceiveCallbackType receiveAudio);
    UNITYEXPORT void UNITYCALLCONV setLibraryPath(const char *path);
    UNITYEXPORT void UNITYCALLCONV setAddressAndPort(
        const char *address,
        int mytcpport,
        int peertcpport,
        int myvideoport,
        int peervideoport,
        int myaudioport,
        int peeraudioport);
    UNITYEXPORT bool UNITYCALLCONV initializeTcpSender();
    UNITYEXPORT bool UNITYCALLCONV initializeTcpReceiver();
    UNITYEXPORT void UNITYCALLCONV closeTcpSocket();

    UNITYEXPORT int UNITYCALLCONV initEncodeVideoData(int videoFormat);
    UNITYEXPORT void UNITYCALLCONV encodeVideoData(unsigned char *inputData, int length);
    UNITYEXPORT void UNITYCALLCONV initEncodeAudioData();
    UNITYEXPORT void UNITYCALLCONV encodeAudioData(const unsigned char *audioData, int);
    UNITYEXPORT void UNITYCALLCONV destroyEncoder();

    UNITYEXPORT void UNITYCALLCONV initDecodeVideoData();
    UNITYEXPORT void UNITYCALLCONV receiveAndDecodeVideoData();
    UNITYEXPORT void UNITYCALLCONV initDecodeAudioData();
    UNITYEXPORT void UNITYCALLCONV receiveAndDecodeAudioData();
    UNITYEXPORT void UNITYCALLCONV destroyDecoder();

    UNITYEXPORT void UNITYCALLCONV test();
};
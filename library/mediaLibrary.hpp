#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

#include "opus/include/opus.h"

typedef void (*ReceiveCallbackType)(unsigned char *, int, int);
typedef void (*CallbackType)(const char *);

extern "C"
{
    UNITYEXPORT void UNITYCALLCONV setCallback(CallbackType debug, ReceiveCallbackType receive);
    UNITYEXPORT void UNITYCALLCONV setLibraryPath(const std::string &path);
    UNITYEXPORT bool UNITYCALLCONV initializeTcpSender();
    UNITYEXPORT bool UNITYCALLCONV initializeTcpReceiver();
    UNITYEXPORT void UNITYCALLCONV closeTcpSocket();
    UNITYEXPORT int UNITYCALLCONV initEncodeVideoData(int videoFormat);
    UNITYEXPORT void UNITYCALLCONV encodeVideoData(unsigned char *inputData, int length);
    UNITYEXPORT void UNITYCALLCONV destroyEncoder();
    UNITYEXPORT void UNITYCALLCONV initDecodeVideoData();
    UNITYEXPORT void UNITYCALLCONV receiveAndDecodeVideoData();
    UNITYEXPORT void UNITYCALLCONV destroyDecoder();
    UNITYEXPORT void UNITYCALLCONV test();
};
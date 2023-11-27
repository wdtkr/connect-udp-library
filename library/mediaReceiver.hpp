#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

#include "opus/include/opus.h"

typedef void (*ReceiveCallbackType)(const char *, ssize_t, int);
typedef void (*CallbackType)(const char *);

extern "C"
{
    UNITYEXPORT int UNITYCALLCONV preDecodeVideoData(unsigned char *inputData, int inputLength, unsigned char *outputData, int outputLength);
    UNITYEXPORT int UNITYCALLCONV decodeVideoData(unsigned char *inputData, int inputLength, unsigned char *outputData, int outputLength);
    UNITYEXPORT int UNITYCALLCONV preDecodeAudioData(unsigned char *inputData, int inputLength, opus_int16 *outputData, int outputLength);
    UNITYEXPORT int UNITYCALLCONV decodeAudioData(unsigned char *inputData, int inputLength, opus_int16 *outputData, int outputLength);
};
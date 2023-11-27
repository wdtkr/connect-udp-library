#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

typedef void (*ReceiveCallbackType)(const char *, ssize_t, int);
typedef void (*CallbackType)(const char *);

extern "C"
{
    UNITYEXPORT void UNITYCALLCONV preSendVideoData(unsigned char *data, int dataLength);
    UNITYEXPORT void UNITYCALLCONV sendVideoData(unsigned char *data, int dataLength);
    UNITYEXPORT void UNITYCALLCONV preSendAudioData(unsigned char *data, int dataLength);
    UNITYEXPORT void UNITYCALLCONV sendAudioData(unsigned char *data, int dataLength);
};
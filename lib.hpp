#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

typedef void (*CallbackType)(const char *);

extern "C"
{
    UNITYEXPORT void UNITYCALLCONV triggerCallback();
    UNITYEXPORT void UNITYCALLCONV setCallback(CallbackType debug, CallbackType start, CallbackType receive);
    UNITYEXPORT void UNITYCALLCONV sendUDPMessage(const char *IP, int port, const char *message);
    UNITYEXPORT void UNITYCALLCONV preReceiveUDPMessage(int port);
    UNITYEXPORT void UNITYCALLCONV receiveUDPMessage();
    UNITYEXPORT void UNITYCALLCONV socketClose();
};
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
    UNITYEXPORT void UNITYCALLCONV setCallback(CallbackType callback);
    UNITYEXPORT void UNITYCALLCONV triggerCallback();
    UNITYEXPORT const char UNITYCALLCONV *registerFqdn(const char *a);
    UNITYEXPORT void UNITYCALLCONV sendUDPMessage(const char *IP, int port, const char *message);
    UNITYEXPORT void UNITYCALLCONV receiveUDPMessage(int port);
    UNITYEXPORT const char UNITYCALLCONV *sendBinaryData(const char *a);
    void waitDataReceive();
    void cppCallBack();
};
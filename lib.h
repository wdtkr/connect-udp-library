#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

extern "C"
{
    UNITYEXPORT int UNITYCALLCONV sum(int a, int b);
    UNITYEXPORT const char UNITYCALLCONV *registerFqdn(const char *a);
    UNITYEXPORT const char UNITYCALLCONV *sendBinaryData(const char *a);
    void waitDataReceive();
    void cppCallBack();
};
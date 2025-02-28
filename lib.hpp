#ifdef _WIN32
#define UNITYCALLCONV __stdcall
#define UNITYEXPORT __declspec(dllexport)
#else
#define UNITYCALLCONV
#define UNITYEXPORT
#endif

#include <unistd.h>

enum class DataType : uint8_t
{
    TEXT_DATA = 1,
    INTEGER_DATA = 2,
    VIDEO_DATA = 3
};

// ReceiveCallbackTypeの引数
// (*ReceiveCallbackType)(データ, データサイズ, データの種類);
// データの種類
// 1: 文字列
// 2: 座標
// 3: ビデオデータ

typedef void (*ReceiveCallbackType)(const char *, ssize_t, int);
typedef void (*CallbackType)(const char *);

extern "C"
{
    UNITYEXPORT void UNITYCALLCONV triggerCallback();
    UNITYEXPORT void UNITYCALLCONV setCallback(CallbackType debug, ReceiveCallbackType receive, CallbackType start);
    UNITYEXPORT void UNITYCALLCONV sendUDPMessage(const char *IP, int port, const char *message);
    UNITYEXPORT void UNITYCALLCONV preReceiveUDPMessage(int port);
    UNITYEXPORT void UNITYCALLCONV receiveUDPMessage();
    UNITYEXPORT void UNITYCALLCONV socketClose();
};
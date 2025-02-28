#include <string>
#include <fstream>
#include <thread>
#include <cstring>      // for strcpy and strlen
#include <cstdlib>      // for exit and EXIT_FAILURE
#include <iostream>     // for cout (optional)
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <unistd.h>     // for close
#include <arpa/inet.h>  // for inet_addr
#include <dlfcn.h>
#include "lib.hpp"

CallbackType debug_callback = nullptr;
ReceiveCallbackType receive_callback = nullptr;
CallbackType start_callback = nullptr;

struct sockaddr_in servaddr;
bool stopThreadFlag = false;
int sockfd;
char buffer[1024];

void triggerCallback()
{
    if (debug_callback != nullptr)
    {
        debug_callback("debugコールバック");
    }
}

void setCallback(CallbackType debug, ReceiveCallbackType receive, CallbackType start)
{
    debug_callback = debug;
    receive_callback = receive;
    start_callback = start;

    triggerCallback();
}

// UDPメッセージを送信する関数
void sendUDPMessage(const char *IP, int port, const char *message)
{
    std::string debug_message_pre = "C++ 送信前段階：" + std::string(message);
    debug_callback(debug_message_pre.c_str());

    // buffer初期化
    memset(buffer, 0, sizeof(buffer));

    int sockfd;                  // ソケットのファイルディスクリプタを格納
    struct sockaddr_in servaddr; // サーバーアドレス情報を格納

    // UDPソケットの作成
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        debug_callback("ソケット作成失敗");
    }

    memset(&servaddr, 0, sizeof(servaddr)); // servaddrを0で初期化

    // サーバーのアドレス情報を設定
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(IP);

    std::string debug_message = "C++ 送信前段階：" + std::string(message);
    debug_callback(debug_message.c_str());

    // メッセージを送信
    if (sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        debug_callback("送信失敗");
    }

    close(sockfd); // ソケットを閉じる
}

// UDPメッセージを受信するための準備を行う関数
// initReceiveUDPMessageとかに変えたいかも。
void preReceiveUDPMessage(int port)
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        debug_callback("socket creation failed 1");
        return;
    }

    // ソケットオプションの設定
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        debug_callback("setsockopt(SO_REUSEADDR) failed");
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        debug_callback("socket creation failed 2");
        return;
    }

    start_callback("正常に受信を開始しました。");
}

// UDPメッセージを受信する関数
void receiveUDPMessage()
{
    // buffer初期化
    memset(buffer, 0, sizeof(buffer));

    debug_callback("receiveUDPMessageが実行開始されました。");
    socklen_t len;
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';

    // receive_callbackのnullptrチェック
    if (receive_callback)
    {
        std::string debug_message = "C++ 受信後段階：" + std::string(buffer);
        debug_callback(debug_message.c_str());
        receive_callback(buffer, n, 0);
    }

    debug_callback("receiveUDPMessageが完了しました。");
}

const char *sendBinaryData(const char *a)
{
    char *pszReturn = new char[strlen(a) + 1];
    strcpy(pszReturn, a);
    return pszReturn;
}

void freeMemory(const char *ptr)
{
    delete[] ptr;
}

// Socketの正常終了
void socketClose()
{
    close(sockfd);
}

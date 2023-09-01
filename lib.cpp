#include <string>
#include <fstream>
#include <cstring>      // for strcpy and strlen
#include <cstdlib>      // for exit and EXIT_FAILURE
#include <iostream>     // for cout (optional)
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <unistd.h>     // for close
#include <arpa/inet.h>  // for inet_addr
#include "lib.h"

std::thread *pThread = nullptr;
CallbackType g_callback = nullptr;

void setCallback(CallbackType callback)
{
    g_callback = callback;
    triggerCallback();
}

void triggerCallback()
{
    if (g_callback != nullptr)
    {
        g_callback("Hello from C++");
    }
}

const char *registerFqdn(const char *a)
{
    const char szSampleString[] = "Hello World";
    char *pszReturn = new char[strlen(szSampleString) + 1];
    strcpy(pszReturn, szSampleString);
    return pszReturn;
}

// UDPメッセージを送信する関数
void sendUDPMessage(const char *IP, int port, const char *message)
{
    int sockfd;                  // ソケットのファイルディスクリプタを格納
    struct sockaddr_in servaddr; // サーバーアドレス情報を格納

    // UDPソケットの作成
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed"); // ソケット作成失敗時のエラーメッセージ
        exit(EXIT_FAILURE);               // プログラムを終了
    }

    memset(&servaddr, 0, sizeof(servaddr)); // servaddrを0で初期化

    // サーバーのアドレス情報を設定
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(IP);

    // メッセージを送信
    if (sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("send failed"); // 送信失敗時のエラーメッセージ
        exit(EXIT_FAILURE);    // プログラムを終了
    }

    close(sockfd); // ソケットを閉じる
}

// UDPメッセージを受信する関数
void receiveUDPMessage(int port)
{
    int sockfd;
    struct sockaddr_in servaddr;

    char buffer[1024];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        socklen_t len;
        int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';

        if (g_callback)
        {
            g_callback(buffer);
        }
    }
}

const char *sendBinaryData(const char *a)
{
    char *pszReturn = new char[strlen(a) + 1];
    strcpy(pszReturn, a);
    return pszReturn;
}

void waitDataReceive()
{
}

void cppCallBack()
{
    // テスト用。後々は、関数を引数に受け取ってから、その関数をコールバックする
}

void freeMemory(const char *ptr)
{
    delete[] ptr;
}
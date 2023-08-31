#include <string>
#include <fstream>
#include "lib.h"

int sum(int a, int b)
{
    return a + b;
}

const char *registerFqdn(const char *a)
{
    const char szSampleString[] = "Hello World";
    char *pszReturn = new char[strlen(szSampleString) + 1];
    strcpy(pszReturn, szSampleString);
    return pszReturn;
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
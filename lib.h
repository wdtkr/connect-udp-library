#include <iostream>
#include <string>

extern "C"
{
    int sum(int a, int b);
    void registerFqdn(std::string a);
    void sendBinaryData();
    void waitDataReceive();
};
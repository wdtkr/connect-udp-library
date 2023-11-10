#include <dlfcn.h>
#include <iostream>

int main()
{
    // ライブラリの読み込み
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        std::cerr << "ライブラリの読み込みに失敗しました: " << dlerror() << std::endl;
        return -1;
    }

    // // ライブラリの関数を取得
    // typedef int (*CreateDecoderFunc)(ISVCDecoder **ppDecoder);
    // CreateDecoderFunc createDecoder = (CreateDecoderFunc)dlsym(handle, "WelsCreateDecoder");
    // if (!createDecoder)
    // {
    //     std::cerr << "関数の取得に失敗しました: " << dlerror() << std::endl;
    //     dlclose(handle);
    //     return -1;
    // }

    std::cout << "ok" << std::endl;
    return 0;
}
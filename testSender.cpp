#include <dlfcn.h>
#include <iostream>

typedef void (*FUNC1)();

int main()
{
    // ライブラリの読み込み
    void *handle = dlopen("../libopenh264-2.3.1-mac-arm64.dylib", RTLD_LAZY);
    if (!handle)
    {
        std::cerr << "ライブラリの読み込みに失敗しました" << dlerror() << std::endl;
        return -1;
    }

    // エンコーダを作成する関数を取得
    typedef int (*CreateEncoderFunc)(void **);
    CreateEncoderFunc createEncoder = (CreateEncoderFunc)dlsym(handle, "WelsCreateEncoder");
    if (!createEncoder)
    {
        std::cerr << "Cannot load symbol 'WelsCreateEncoder': " << dlerror() << '\n';
        dlclose(handle);
        return 1;
    }

    // エンコーダのインスタンスを作成
    void *encoder = nullptr;
    if (createEncoder(&encoder) != 0)
    {
        std::cerr << "Failed to create encoder" << '\n';
        dlclose(handle);
        return 1;
    }
}
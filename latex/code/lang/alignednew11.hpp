#include <cstddef>  // for std::size_t
#include <cstring>
#if __STDC_VERSION >= 201112L
#include <stdlib.h> // for aligned_alloc()
#else
#include <malloc.h> // for _aligned_malloc() or memalign()
#endif

struct alignas(32) MyType32 {
    int i;
    char c;
    std::string s[4];
    ...
    static void* operator new (std::size_t size) {
        // 按照请求的对齐分配内存：
#if __STDC_VERSION >= 201112L
        // 使用C11的API：
        return aligned_alloc(alignof(MyType32), size);
#else
#ifdef _MSC_VER
        // 使用Windows的API：
        return _aligned_malloc(size, alignof(MyType32));
#else
        // 使用Linux的API：
        return memalign(alignof(MyType32), size);
#endif
#endif
    }

    static void operator delete (void* p) {
        // 释放对齐分配的内存：
#ifdef _MSC_VER
        // 使用Windows的特殊API：
        _aligned_free(p);
#else
        // C11/Linux可以使用通用的free()：
        free(p);
#endif
    }

    // 自从C++14起：
    static void operator delete (void* p, std::size_t size) {
        MyType32::operator delete(p);   // 使用无大小的delete
    }
    ...
    // 定义数组需要的new[]和delete[]
}
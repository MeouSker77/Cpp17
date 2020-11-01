#ifndef TRACKNEW_HPP
#define TRACKNEW_HPP

#include <new>      // for std::align_val_t
#include <cstdio>   // for printf()
#include <cstdlib>  // for malloc()和aligned_alloc()
#ifdef _MSC_VER
#include <malloc.h> // for _aligned_malloc()和_aligned_free()
#endif

class TrackNew {
private:
    static inline int numMalloc = 0;    // malloc调用的次数
    static inline size_t sumSize = 0;   // 总共分配的字节数
    static inline bool doTrace = false; // 开启追踪
    static inline bool inNew = false;   // 不追踪new重载里的输出
public:
    static void reset() {               // 重置new/memory计数器
        numMalloc = 0;
        sumSize = 0;
    }

    static void trace(bool b) {         // 开启/关闭trace
        doTrace = b;
    }

    //  被追踪的分配内存的实现：
    static void* allocate(std::size_t size, std::size_t align, const char* call) {
        // 追踪内存分配：
        ++numMalloc;
        sumSize += size;
        void* p;
        if (align == 0) {
            p = std::malloc(size);
        }
        else {
#ifdef _MSC_VER
            p = _aligned_malloc(size, align);       // Windows API
#else
            p = std::aligned_alloc(align, size);    // C++17 API
        }
        if (doTrace) {
            // 不要在这里使用std::cout因为它可能当我们在处理内存分配时
            // 分配内存（最好情况也是core dump）
            printf("#%d %s ", numMalloc, call);
            printf("(%zu bytes, ", size);
            if (align > 0) {
                printf("%zu-byte aligned) ", align);
            } else {
                printf("def-aligned) ");
            }
            printf("=> %p (total: %zu bytes)\n", (void *) p, sumSize);
        }
        return p;
    }

    static void status() {  // 打印当前的状态
        printf("%d allocations for %zu bytes\n", numMalloc, sumSize);
    }
};

[[nodiscard]]
void* operator new (std::size_t size) {
    return TrackNew::allocate(size, 0, "::new");
}

[[nodiscard]]
void* operator new (std::size_t size, std::align_val_t align) {
    return TrackNew::allocate(size, static_cast<std::size_t>(align), "::new aligned");
}

[[nodiscard]]
void* operator new[] (std::size_t size) {
    return TrackNew::allocate(size, 0, "::new[]");
}

[[nodiscard]]
void* operator new[] (std::size_t size, std::align_val_t align) {
    return TrackNew::allocate(size, static_cast<std::size_t>(align), "::new[] aligned");
}

// 确保释放操作匹配
void operator delete (void* p) noexcept {
    std::free(p);
}
void operator delete (void* p, std::size_t) noexcept {
    ::operator delete(p);
}
void operator delete (void* p, std::align_val_t) noexcept {
#ifdef  _MSC_VER
    _aligned_free(p);   // Windows API
#else
    std::free(p);       // C++17 API
}
void operator delete (void* p, std::size_t, std::align_val_t align) noexcept {
    ::operator delete(p, align);
}

#endif  // TRACKNEW_HPP
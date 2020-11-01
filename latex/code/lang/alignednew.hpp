#include <cstddef>  // for std::size_t
#include <new>      // for std::align_val_t
#include <cstdlib>  // for malloc(), aligned_alloc(), free()
#include <cstring>

struct alignas(32) MyType32 {
    int i;
    char c;
    std::string s[4];
    ...
    static void* operator new (std::size_t size) {
        // 调用new获取默认对齐的数据：
        std::cout << "MyType32::new() with size " << size << '\n';
        return ::operator new(size);
    }
    static void* operator new (std::size_t size, std::align_val_t align) {
        // 调用new获取超对齐数据：
        std::cout << "MyType32::new() with size " << size
                  << " and alignment " << static_cast<std::size_t>(align)
                  << '\n';
        return ::operator new(size, align);
    }

    static void operator delete (void* p) {
        // 调用delete释放默认对齐的数据：
        std::cout << "MyType32::delete() without alignment\n";
        ::operator delete(p);
    }
    static void operator delete (void* p, std::size_t size) {
        MyType32::operator delete(p);   // 使用无大小的delete
    }
    static void operator delete (void* p, std::align_val_t align) {
        // 调用delete释放超对齐的数据：
        std::cout << "MyType::32::delete() with alignment\n";
        ::operator delete(p, align);
    }
    static void operator delete (void* p, std::size_t size, std::align_val_t align) {
        MyType32::operator delete(p, align);    // 使用无大小的delete
    }

    // 定义数组需要的operator new[]和operator delete[]
    ...
}
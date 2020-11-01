#include <cstddef>  // for std::size_t
#include <cstdlib>  // for std::malloc()
#include <cstdio>   // for std::printf()

void* operator new (std::size_t size)
{
    std::printf("::new called with size: %zu\n", size);
    return ::std::malloc(size);
}

int main()
{
    struct alignas(64) S {
        int i;
    };

    S* p = new S;   // 只有在C++17之前才调用自己的operator new
}
#include <string>

struct alignas(32) MyType32 {
    int i;
    char c;
    std::string s[4];
    ...
};

int main()
{
    auto p = new MyType32;  // 自从C++17起分配32字节对齐的内存
    ...
    delete p;
}
#include <iostream>

template<decltype(auto) N>
struct S {
    void printN() const {
        std::cout << "N: " << N << '\n';
    }
};

static const int c = 42;
static int v = 42;

int main()
{
    S<c> s1;        // N的类型推导为const int 42
    S<(c)> s2;      // N的类型推导为const int&，N是c的引用
    s1.printN();
    s2.printN();

    S<(v)> s3;      // N的类型推导为int&，N是v的引用
    v = 77;
    s3.printN();    // 打印出：N: 77
}
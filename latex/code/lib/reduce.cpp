#include <array>
#include <iostream>
#include <numeric>

int main()
{
    std::array coll{3, 1, 7, 5, 4, 1, 6, 3};

    // 计算元素之和
    std::cout << "sum: "
              << std::reduce(coll.cbegin(), coll.cend())    // 范围
              << '\n';

    // 指定初始值计算元素之和
    std::cout << "sum: "
              << std::reduce(coll.cbegin(), cool.cend()),   // 范围
                             0)                             // 初始值
              << '\n';

    // 处理所有元素的积
    std::cout << "product: "
              << std::reduce(coll.cbegin(), coll.cend(),    // 范围
                             1LL,                           // 初始值
                             std::multiplies{})             // 操作数
              << '\n';

    // 处理所有元素的积（0作为初始值）
    std::cout << "product: "
              << std::reduce(coll.cbegin(), coll.cend(),    // 范围
                             0,                             // 初始值
                             std::multiplies{})             // 操作数
              << '\n';
}
#include <array>
#include <iostream>
#include <numeric>

int main()
{
    std::array coll{3, 1, 7, 5, 4, 1, 6, 3};

    auto twice = [] (int v) { return v*2; };

    // 计算每个元素的两倍的和：
    std::cout << "sum of doubles: "
              << std::transform_reduce(coll.cbegin(), coll.cend(),  // 范围
                                       0, std::plus{}, twice)
              << '\n';

    // 计算每个元素的平方的和：
    std::cout << "sum of squared: "
              << std::transform_reduce(coll.cbegin(), coll.cend(),  // 范围
                                       0L, std::plus{},
                                       [] (auto v) {
                                           return v * v;
                                       })
              << '\n';
}
#include "charconv.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>

int main()
{
    std::vector<double> coll{0.1, 0.3, 0.00001};

    // 创建两个有微小不同的浮点数：
    auto sum1 = std::accumulate(coll.begin(), coll.end(), 0.0, std::plus<>());
    auto sum2 = std::accumulate(coll.rbegin(), coll.rend(), 0.0, std::plus<>());

    // 看起来相同：
    std::cout << "sum1:  " << sum1 << '\n';
    std::cout << "sum2:  " << sum2 << '\n';

    // 但事实上不同：
    std::cout << std::boolalpha << std::setprecision(20);
    std::cout << "equal: " << (sum1 == sum2) << '\n';   // false!!
    std::cout << "sum1:  " << sum1 << '\n';
    std::cout << "sum2:  " << sum2 << '\n';
    std::cout << '\n';

    // 检查双向转换
    d2str2d(sum1);
    d2str2d(sum2);
}
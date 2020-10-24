#include <array>
#include <iostream>
#include <numeric>
#include <string>

int main()
{
    std::array coll1{3, 1, 7, 5, 4, 1, 6, 3};
    std::array coll2{1, 2, 3, 4, 5, 6, 7, 8};

    // 计算每个元素的平方的和：
    std::cout << "sum of squared:         "
              << std::transform_reduce(coll1.cbegin(), coll1.cend(),    // 第一个范围
                                       coll1.cbegin(),                  // 第二个范围
                                       0L)
              << '\n';

    // 计算每两个相应元素的差的乘积：
    std::cout << "product of differences: "
              << std::transform_reduce(coll1.cbegin(), coll1.cend(),    // 第一个范围
                                       coll2.cbegin(),                  // 第二个范围
                                       1L,
                                       std::multiplies{}, std::minus{})
              << '\n';

    // 先把两个相应元素转换成的字符串连接起来，再连接所有上一步生成的字符串：
    std::cout << "sum of combined digits: "
              << std::transform_reduce(coll1.cbegin(), coll1.cend(),    // 第一个范围
                                       coll2.cbegin(),                  // 第二个范围
                                       std::string{}, std::plus{},
                                       [] (auto x, auto y) {
                                           return std::to_string(x) + std::to_string(y) + " ";
                                       })
              << '\n';
}
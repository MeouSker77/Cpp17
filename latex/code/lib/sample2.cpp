#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>    // for sample()
#include <random>       // for 随机数设备和随机数引擎

int main()
{
    // 初始化一个有10,000个字符串的vector
    std::vector<std::string> coll;
    for (int i = 0; i < 10000; ++i) {
        coll.push_back("value" + std::to_string(i));
    }

    // 用一个随机数种子初始化一个Mersenne Twister引擎：
    std::random_device rd;      // 随机数种子（如果支持的话）
    std::mt19937 eng{rd()};     // Mersenne Twister引擎

    // 初始化目标区间（至少要能存放10个元素）：
    std::vector<std::string> subset;
    subset.resize(100);

    // 从源区间随机拷贝10个元素到目的区间：
    auto end = std::sample(coll.begin(), coll.end(),
                           subset.begin(),
                           10,
                           eng);

    // 打印被提取出的元素（使用返回值作为终点）：
    std::for_each(subset.begin(), end,
                  [] (const auto& s) {
                      std::cout << "random elem: " << s << '\n';
                  });
}
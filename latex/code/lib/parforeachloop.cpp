#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <execution>    // for 执行策略
#include <cstdlib>      // for atoi()
#include "timer.hpp"

int main(int argc, char* argv[])
{
    // 从命令行读取numElems（默认值：1000）
    int numElems = 1000;
    if (argc > 1) {
        numElems = std::atoi(argv[1]);
    }

    struct Data {
        double value;   // 初始值
        double sqrt;    // 并行计算平方根
    };

    // 初始化numElems个还没有计算平方根的值：
    std::vector<Data> coll;
    coll.reserve(numElems);
    for (int i = 0; i < numElems; ++i) {
        coll.push_back(Data{i * 4.37, 0});
    }

    // 循环来重复测量
    for (int i{0}; i < 5; ++i) {
        Timer t;
        // 顺序执行：
        for_each(std::execution::seq,
                 coll.begin(), coll.end(),
                 [] (auto& val) {
                     val.sqrt = std::sqrt(val.value);
                 });
        t.printDiff("sequential: ");

        // 并行执行
        for_each(std::execution::par,
                 coll.begin(), coll.end(),
                 [] (auto& val) {
                     val.sqrt = std::sqrt(val.value);
                 });
        t.printDiff("parallel: ");
        std::cout << '\n';
    }
}

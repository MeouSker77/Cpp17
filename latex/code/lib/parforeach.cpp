#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <execution>    // for执行策略
#include <cmath>        // for math()
#include "timer.hpp"

int main()
{
    int numElems = 1000;

    struct Data {
        double value;   // 初始值
        double sqrt;    // 并行计算平方根
    };

    // 初始化numElems个还没有计算平方根的值
    std::vector<Data> coll;
    coll.reserve(numElems);
    for (int i=0; i<numElems; ++i) {
        coll.push_back(Data{i * 4.37, 0});
    }

    // 并行计算平方根
    for_each(std::execution::par,
             coll.begin(), coll.end(),
             [](auto& val) {
                val.sqrt = std::sqrt(val.value);
            });
}
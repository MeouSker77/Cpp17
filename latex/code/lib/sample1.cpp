#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>    // for sample()
#include <random>       // for default_random_engine

int main()
{
    // 初始化一个有10,000个字符串的vector：
    std::vector<std::string> coll;
    for (int i = 0; i < 10000; ++i) {
        coll.push_back("value" + std::to_string(i));
    }

    // 打印10个从集合中随机抽取的元素：
    std::sample(coll.begin(), coll.end(),
                std::ostream_iterator<std::string>{std::cout, "\n"},
                10,
                std::default_random_engine{});
}
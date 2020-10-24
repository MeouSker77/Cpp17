#include <iostream>
#include <vector>
#include <string>
#include <algorithm>    // for for_each_n()

int main()
{
    // 初始化10,000个string的vector：
    std::vector<std::string> coll;
    for (int i = 0; i < 10000; ++i) {
        coll.push_back(std::to_string(i));
    }

    // 修改前5个元素：
    for_each_n(coll.begin(), 5,
               [] (auto& elem) {
                   elem = "value" + elem;
               });

    // 打印前10个元素：
    for_each_n(coll.begin(), 10,
               [] (const auto& elem) {
                   std::cout << elem << '\n';
               });
}
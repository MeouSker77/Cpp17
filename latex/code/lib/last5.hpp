#ifndef LAST5_HPP
#define LAST5_HPP

#include <iterator>
#include <iostream>

template<typename T>
void printLast5(const T& coll)
{
    // 计算大小：
    auto size{std::size(coll)};
    // 把迭代器递增到倒数第五个元素处
    std::cout << size << " elems: ";
    auto pos{std::begin(coll)};
    if (size > 5) {
        std::advance(pos, size - 5);
        std::cout << "... ";
    }
    // 打印出剩余的元素：
    for (; pos != std::end(coll); ++pos) {
        std::cout << *pos << ' ';
    }
    std::cout << '\n';
}

#endif  // LAST5_HPP
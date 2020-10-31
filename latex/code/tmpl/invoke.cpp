#include "invoke.hpp"
#include <iostream>
#include <vector>

void print(const std::vector<int>& coll)
{
    std::cout << "elems: ";
    for (const auto& elem : coll) {
        std::cout << elem << ' ';
    }
    std::cout << '\n';
}

int main()
{
    std::vector<int> vals{0, 8, 15, 42, 13, -1, 0};

    call([&vals] {
             std::cout << "size: " << vals.size() << '\n';
         });
    call(print, vals);

    call(&decltype(vals)::pop_back, vals);
    call(print, vals);

    call(&decltype(vals)::clear, vals);
    call(print, vals);
}
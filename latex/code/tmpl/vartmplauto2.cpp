#include "vartmplauto.hpp"
#include <iostream>

void printArr()
{
    std::cout << "arr<int, 5>:  ";
    for (const auto& elem : arr<int, 5>) {
        std::cout << elem << ' ';
    }
    std::cout << "\narr<int, 5u>: ";
    for (const auto& elem : arr<int, 5u>) {
        std::cout << elem << ' ';
    }
    std::cout << '\n';
}
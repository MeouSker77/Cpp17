#include <iostream>

template<auto Sep = ' ', typename First, typename... Args>
void print(const First& first, const Args&... args) {
    std::cout << first;
    auto outWithSep = [] (const auto& arg) {
                          std::cout << Sep << arg;
                      };
    (... , outWithSep(args));
    std::cout << '\n';
}
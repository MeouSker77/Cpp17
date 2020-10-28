#include <iostream>
#include <string>
#include <map>

int main()
{
    std::map<int, std::string> m{{1, "mango"},
                                 {2, "papaya"},
                                 {3, "guava"}};

    auto nh = m.extract(2); // nh的类型为decltype(m)::node_type
    nh.key() = 4;
    m.insert(std::move(nh));

    for (const auto& [key, value] : m) {
        std::cout << key << ": " << value << '\n';
    }
}
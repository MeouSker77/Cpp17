#include <string>
#include <iostream>

struct MyData {
    inline static std::string gName = "global";             // 整个程序中只有一个
    inline static thread_local std::string tName = "tls";   // 每个线程有一个
    std::string lName = "local";                            // 每个实例有一个
    ...
    void print(const std::string& msg) const {
        std::cout << msg << '\n';
        std::cout << "- gName: " << gName << '\n';
        std::cout << "- tName: " << tName << '\n';
        std::cout << "- lName: " << lName << '\n';
    }
};

inline thread_local MyData myThreadData;    // 每个线程一个对象
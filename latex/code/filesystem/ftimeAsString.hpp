#include <string>
#include <chrono>
#include <filesystem>

std::string asString(const std::filesystem::file_time_type &ft)
{
    using system_clock = std::chrono::system_clock;
    auto t = system_clock::to_time_t(system_clock::now()
                                     + (ft - std::filesystem::file_time_type::clock::now()));
    // 转换为日历时间(跳过末尾的换行符）：
    std::string ts = ctime(&t);
    ts.resize(ts.size()-1);
    return ts;
}
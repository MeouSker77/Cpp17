#include <string>

template <typename T>
std::string asString(T x)
{
    if constexpr(std::is_same_v<T, std::string>) {
        return x;                   // 如果T不能自动转换为string该语句将无效
    }
    else if constexpr(std::is_arithmetic_v<T>) {
        return std::to_string(x);   // 如果T不是数字类型该语句将无效
    }
    else {
        return std::string(x);      // 如果不能转换为string该语句将无效
    }
}
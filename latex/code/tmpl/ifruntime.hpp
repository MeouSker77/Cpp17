#include <string>

template <typename T>
std::string asString(T x)
{
    if (std::is_same_v<T, std::string>) {
        return x;                   // 如果不能自动转换为string会导致ERROR
    }
    else if (std::is_numeric_v<T>) {
        return std::to_string(x);   // 如果不是数字将导致ERROR
    }
    else {
        return std::string(x);      // 如果不能转换为string将导致ERROR
    }
}

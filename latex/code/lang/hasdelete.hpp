#include <type_traits>  // for true_type, false_type, void_t

// 主模板
template<typename, typename = std::void_t<>>
struct HasDelete : std::false_type {
};

// 部分特化版（可能被SFINE'd away）：
template<typename T>
struct HasDelete<T, std::void_t<decltype(T::operator delete(nullptr))>> : std::true_type  {
};
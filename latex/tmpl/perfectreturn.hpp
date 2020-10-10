#include <function>     // for std::forward()
#include <type_traits>  // for std::is_same<> and std::invoke_result<>

template<typename Callable, typename... Args>
decltype(auto) call(Callable op, Args&... args)
{
    if constexpr(std::is_void_v<
                 std::invoke_result_t<Callable, Args...>>) {
        // 返回值类型是void：
        op(std::forward<Args>(args)...);
        ... // 在返回前进行一些处理
        return;
    }
    else {
        // 返回值类型不是void:
        decltype(auto) ret{op(std::forward<Args>(args)...)};
        ... // 在返回前用ret进行一些处理
        return ret;
    }
}

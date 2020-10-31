#include <utility>      // for std::invoke()
#include <functional>   // for std::forward()

template<typename Callable, typename... Args>
void call(Callable&& op, Args&&... args)
{
    ...
    std::invoke(std::forward<Callable>(op),     // 调用传入的可调用对象
                std::forward<Args>(args)...);   // 以传入的其他参数为参数
}
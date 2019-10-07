# Chapter 9 编译期if语句

通过使用if constexptr(...)语句，编译期可以根据这个这个编译期的表达式来推断是否要编译then的部分还是else的部分。其他部分都会被丢弃，所以不会生成代码。这并不意味着被丢弃的部分会被完全忽略，它仍然会被检查是否正确，就像没被使用的模板一样。

例如：

*tmpl/ifcomptime.hpp*

```cpp
#include <string>

template <typename T>
std::string asString(T x)
{
    if constexpr(std::is_same_v<T, std::string>) {
        return x;   //如果T不是string就是无效的语句
    }
    else if constexpr(std::is_arithmetic_v<T>) {
        return std::to_string(x);   //如果x不是数字就是无效的语句
    }
    else {
        return std::string(x);  //如果不能转换为string就是无效的语句。
    }
}
```

这里，我们使用了这个特性来在编译期决定我们是只返回传递进的字符串，还是对传递进的数字调用std::to_string()，还是尝试将传递进的参数转换为std::string。因为无效的调用都会被丢弃，因此下面的代码能正常编译（如果使用普通的if语句就不会正常编译）：

*templ/ifcomptime.cpp*

```cpp
#include "ifcomptime.hpp"
#include <iostream>

int main()
{
    std::cout << asString(42) << '\n';
    std::cout << asString(std::string("hello")) << '\n';
    std::cout << asString("hello") << '\n';
}
```

## 9.1 编译期if语句产生的原因

如果我们使用普通的运行期if语句:

*templ/ifruntime.hpp*

```cpp
template <typename T>
std::string asString(T x)
{
    if (std::is_save_v<T, std::string>) {
        return x;   //如果x不能转换为string就错误
    }
    else if (std::is_numeric_v<T>) {
        return std::to_string(x);   //如果x不是数字就错误
    }
    else {
        return std::string(x);  //如果x不能转换为string就错误
    }
}
```

上边的代码永远不能通过编译，这是函数模板要么不编译要么作为一个整体编译的规则的结果。if语句中的检查是一个运行时的特性。尽管在编译期间我们就能肯定if语句中条件的结果肯定是false，但then中的部分也必须要能通过编译。所以，当传递进一个std::string或者字符串字面量的时候，会因为没有相应的std::to_string()导致编译失败。如果我们传递一个普通的数字类型的值，编译也会失败，因为第一和第三个返回语句中都不能将数字转换为string，所以都是无效的。

现在通过使用编译期if语句，没有用到的其他部分就会被丢弃：

* 当传递进std::string值的时候，else if和else部分都会被丢弃。
* 当传递进数字值的时候，if和else部分都会被丢弃。
* 当传递经字符串字面量的时候（例如，类型const char*），if和else if部分会被丢弃

因此，每一个无效的部分在编译期间都会被丢掉，所以代码能正确编译。

注意被丢弃的部分并不是被忽略了，只是没有实例化。这些部分的语法必须是正确的而且和模板参数无关的调用必须是有效的。事实上，编译的第一个阶段仍然会执行，这个阶段会检查语法和所有不依赖模板参数的词法单元。所有的static_assert也必须是正确的，就算它们处在会不会被编译的分支里。

例如：

```cpp
template<typename T>
void foo(T t)
{
    if constexpr(std::is_integral_v<T>) {
        if (t > 0) {
            foo(t-1);   //OK
        }
    }
    else {
        undeclared(t);  //如果该函数没有被声明且else分支没有被丢弃（即T不是整数时）会编译错误
        undeclared();   //如果该函数没有被声明则编译错误（即使else分支被丢弃也一样）
        static_assert(false, "no integral");    //总是会进行断言（即使else分支被丢弃也一样）
    }
}
```

对于一个正确的编译器来说，这段代码不能通过编译有两个原因：

* 即使T是整数类型，如果函数没有声明，那么被丢弃的else部分中

```cpp
undeclared();   //如果该函数没有被声明则编译错误（即使else分支被丢弃也一样）
```

的调用也会产生错误，因为这个调用和模板参数无关。

* 即使处于被丢弃的else部分，

```cpp
static_assert(false, "no integral");    ////总是会进行断言（即使else分支被丢弃也一样）
```

的调用也总是会失败，因为这个调用也不依赖模板参数。一个满足相同功能的编译期条件断言应该是：

```cpp
static_assert(!std::is_integral_v<T>, "no integral");
```

注意有些编译器（例如Visual C++ 2013和2015)并没有正确的实现模板编译的两个阶段。它们把第一阶段的大部分工作都推迟到了第二阶段所以一些无效的调用甚至一些错误的语句也可能通过编译。

## 9.2 使用编译期if

原则上讲，你可以像运行时if语句一样使用编译期if语句，只要条件表达式是一个编译期语句。你甚至可以混合使用两种两种if语句：

```cpp
if constexpr (std::is_integral_v<std::remove_reference_t<T>>) {
    if (val > 10) {
        if constexpr (std::numeric_limits<char>::is_signed) {
            ...
        }
        else {
            ...
        }
    }
    else {
        ...
    }
}
else {
    ...
}
```

注意你不可以在函数体外使用if constexpr。因此，你布恩那个使用它替换传统的预处理语句。

### 9.2.1 关于编译期if语句的警告

只要可能就尽量使用编译期if语句，即使它可能会导致一些不太明显的结果，像接下来的小节要探讨的。

#### 编译期if语句影响返回值类型

编译期if语句可能会影响函数的返回值类型。例如，下面的代码能通过编译，但返回值类型可能不同：

```cpp
auto foo()
{
    if constexpr (sizeof(int) > 4) {
        return 42;
    }
    else {
        return 42u;
    }
}
```

这里，因为我们使用了auto，所以函数的返回值类型取决于返回语句，返回语句有依赖于int的大小：

* 如果int大于四字节，那么唯一有效的返回语句将返回42，所以返回类型是int。
* 否则，唯一有效的返回语句返回42u，因此返回类型是unsigned int。

这种情况下带有if constexpr的函数的返回类型可能不同。例如，如果我们省略了eles部分，那么下面的函数的返回类型将是int或void：

```cpp
auto foo()
{
    if constexpr (sizeof(int) > 4) {
        return 42;
    }
}
```

注意如果这里使用运行时if的话这个函数将不能编译，因为返回值类型的推导将会产生歧义。

#### 即使then部分中就返回了也要有else部分

有一种运行时if语句的模式不适用于编译期if语句：如果then部分和else部分都有返回语句那么你可以在运行时if中跳过else关键字。也就是说，例如：

```cpp
if (...) {
    return a;
}
else {
    return b;
}
```

你总是可以写成：

```cpp
if (...) {
    return a;
}
return b;
```

这种模式并不适用于编译期if，因为第二种情况下函数的返回类型将同时依赖两条返回语句，这就会导致问题。例如，修改上例中的代码将导致可能能通过，也可能不能通过编译：

```cpp
auto foo()
{
    if constexpr (sizeof(int) > 4) {
        return 42;
    }
    return 42u;
}
```

如果条件为真（即int大于4字节），那么编译期会推断出两种不同的返回类型，这是无效的。否则，我们就只有一条返回语句，因此代码将能通过编译。

#### 编译期条件语句不支持短路求值

考虑如下代码：

```cpp
template<typename T>
constexpr auto foo(const T& val)
{
    if constexpr (std::is_integral<T>::value) {
        if constexpr (T{} < 10) {
            return val * 2;
        }
    }
    return val;
}
```

这里，我们使用了两次编译期条件来决定直接返回参数的值还是返回它的两倍。它对下面的语句都能正常编译：

```cpp
constexpr auto x1 = foo(42);    //返回84
constexpr auto x2 = foo("hi");  //OK，返回"hi"
```

考虑运行时if语句的短路求值特性。你可能会期望对下面的编译期if语句也会产生相同的结果：

```cpp
template<typename T>
constexpr auto bar(const T& val)
{
    if constexpr (std::is_integral<T>::value && T{} < 10) {
        return val*2;
    }
    return val;
}
```

然而，编译期if的条件语句总是要实例化并且总是要整个进行求值，所以传递一个不支持与10比较的类型将会导致编译错误：

```cpp
constexpr auto x2 = bar("hi");  //编译期错误
```

因此，编译期if没有短路求值的特性。如果某一个编译期条件语句的有效性依赖于之前的编译期条件，那么你必须像第一个foo()中做的那样。作为另一个例子，你可以写：

```cpp
if constexpr (std::is_same_v<MyType, T>) {
    if constexpr (T::i == 42) {
        ...
    }
}
```

而不是：

```cpp
if constexpr (std::is_same_v<MyType, T> && T::i == 42) {
    ...
}
```

### 9.2.2 其他的编译期if示例

#### 泛型值的完美转发

编译期if的一个应用就是先对参数进行处理，然后对返回值进行完美转发。因为decltype(auto)不能推导为void类型（因为void是一个不完全的类型），所以你必须像下面这样写。

*tmpl/prefectreturn.hpp*

```cpp
#include <functional>   //for std::forward()
#include <type_traits>  //for std::is_same<> and std::invoke_result<>

template<typename Callable, typename... Args>
decltype(auto) call(Callable op, Args&&... args) {
    if constexpr(std::is_void_v<std::invoke_result_t<Callable, Args...>>) {
        //返回类型为void:
        op(std::forward<Args>(args)...);
        ...//在返回之前做一些事
        return;
    }
    else {
        //返回类型不是void
        decltype(auto) ret{op(std::forward<Args>(args)...)};
        ...//在返回之前做一些事
        return ret;
    }
}
```

#### 在类型分发中使用编译期if

编译期if的一个典型应用是类型分发。在C++17之前，你必须为每一个你想处理的类型单独重载一个函数版本。现在，有了编译期if语句你可以将所有的逻辑集中到一个函数中：

例如，不在使用如下的重载std::advance()算法的方式：

```cpp
template<typename Iterator, typename Distance>
void advace(Iterator& pos, Distance n) {
    using cat = std::iterator_traits<Iterator>::iterator_category;
    advaceImpl(ops, n, cat);    //迭代器类型的类型分发
}

template<typename Iterator, typename Distance>
void advanceImpl(Iterator& pos, Distance n, std::random_access_iterator_tag) {
    pos += n;
}

template<typename Iterator, typename Distance>
void advanceImpl(Iterator& pos, Distance n, std::bidirectional_iterator_tag) {
    if (n >= 0) {
        while (n--) {
            ++pos;
        }
    }
    else {
        while (n++) {
            --pos;
        }
    }
}

template<typename Iterator, typename Distance>
void advanceImpl(Iterator& pos, Distance n, std::input_iterator_tag) {
    while (n--) {
        ++pos;
    }
}
```

现在我们可以将所有的实现集中在一个函数中：

```cpp
template<typename Iterator, typename Distance>
void advance(Iterator& pos, Distance n) {
    using cat = std::iterator_traits<Iterator>::iterator_category;

    if constexpr (std::is_same_v<cat, std::random_access_iterator_tag>) {
        pos += n;
    }
    else if constexpr (std::is_same_v<cat, std::bidirectional_access_iterator_tag>) {
        if (n >= 0) {
            while (n--) {
                ++pos;
            }
        }
        else {
            while (n++) {
                --pos;
            }
        }
    }
    else {  //input_iterator_tag
        while (n--) {
            ++pos;
        }
    }
}
```

因此，在某种意义上，我们现在有了一个编译期的swtich，它通过编译期if的的条件语句来到达不同的case。然而，注意有一点是不同的：

* 重载函数集合的方式拥有**最佳匹配**的语义
* 编译期if的方式拥有**最先匹配**的语义

另一个类型分发的例子是为get<>()使用编译期if（见1.3节）来实现一个结构化绑定的接口。

第三个例子实在泛型lambda中处理不同的类型例如std::variant<>（见15.2.3）。

## 9.3 带初始化的编译期if

注意编译期if也可以使用新的带初始化的形式（见第2章）。例如，如果有一个constexpr函数foo()，你可以使用：

```cpp
template<typename T>
void bar(const T x)
{
    if constexpr (auto obj = foo(x); std::is_same_v<decltype(obj), T>) {
        std::cout << "foo(x) yields same type\n";
        ...
    }
    else {
        std::cout << "foo(x) yields different type\n";
        ...
    }
}
```

如果有一个constexpr函数foo()的话，你可以使用这种方式来根据foo(x)返回的不同类型来进行不同的处理。

为了根据foo(x)的返回值来进行决定，你可以写：

```cpp
constexpr auto c = ...;
if constexpr (constexpr auto obj = foo(c); obj == 0) {
    std::cout << "foo() == 0\n";
    ...
}
```

注意现在为了在条件语句中使用obj的值必须将它声明为constexpr。

## 9.4 在模板之外使用编译期if

if constexpr可以被用于任何函数，并不仅限于模板。我们只需要一个编译期的条件语句。然而，在这种情况下then和else部分都需要是有效的即使会被丢弃。

例如，下面的代码编译总会失败，因为undeclared()的调用必须是有效的，即使字符是有符号的导致else部分被丢弃：

```cpp
#include <limits>

template<typename T>
void foo(T t);

int main()
{
    if constexpr(std::numeric_limits<char>::is_signed) {
        foo(42);    //OK
    }
    else {
        undeclared(42); //总是错误（即使被丢弃）
    }
}
```

下面的代码也从来不可能成功通过编译，因为总有一一个静态断言会失败：

```cpp
if constexpr(std::numeric_limits<char>::is_signed) {
    static_assert(std::numeric_limits<char>::is_signed);
}
else {
    static_assert(!std::numeric_limits<char>::is_sigend);
}
```

在泛型代码之外使用编译期if的（唯一）好处是被丢弃的代码，尽管必须有效，不会变成最终的可执行程序的一部分，这减小了生成的程序的体积。例如，在这段程序中：

```cpp
#include <limits>
#include <string>
#include <array>

int main()
{
    if (!std::numeric_limits<char>::is_signed) {
        static std::array<std::string, 1000> arr1;
        ...
    }
    else {
        static std::array<std::string, 1000> arr2;
        ...
    }
}
```

要么arr1要么arr2将成为最终程序的一部分，但不可能两者都是。

## 9.5 后记

编译期if语句的动机最早起源于Walter Bright, Herb Sutter和Andrei Alexandrescu在[https://wg21.link/n3329](https://wg21.link/n3329)上和Ville Voutilainen在[https://wg21.link/n4461](https://wg21.link/n4461)上提出的静态if语言特性。在[https://wg21.link/p0128r0](https://wg21.link/p0128r0)上Ville Voutilainen第一次以constexpr_if的名称提出了这个特性（这个特性因此得名）。最后被接受的正式提案由Jens Maurer在[https://wg21.link/p0292r2](https://wg21.link/p0292r2)上发表。

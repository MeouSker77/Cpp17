# Chapter 12 模板参数中的`auto`类型占位符

自从C++17开始你可以使用占位符类型(`auto`和的`decltype(auto)`)作为非模板参数的类型。这也就意味着，我们可以为不同的非模板参数的类型写出泛型的代码。

## 12.1 使用`auto`作为模板参数

自从C++17开始，你可以使用`auto`来声明非类型模板参数。例如：

```cpp
template<auto N>
class S {
    ...
};
```

这允许我们使用不同的参数类型来实例化非类型参数`N`：

```cpp
S<42> s1;       //OK:N的类型为int
S<'a'> s2;      //OK:N的类型为char
```

然而，你不能使用这个特性来将模板参数不允许的类型作为参数：

```cpp
S<2.5> s3;      //错误：非类型模板参数不能为double类型
```

我们甚至可以定义一个特定的类型作为部分特化版：

```cpp
template<int N>
class S {
    ...
};
```

这个特性也支持类模板参数推导（见第8章）。例如：

```cpp
template<typename T, auto N>
class A {
  public:
    A(const std::array<T, N>&) {
    }
    A(T(&)[N]) {
    }
    ...
};
```

这个类可以推导出`T`的类型，`N`的类型和`N`的值：

```cpp
A a2{"hello"};      //OK，推导出A<const char, 6>， N为int类型

std::array<double, 10> sa1;
A a1{sa1};          //OK，推导出A<double, 10>，N为std::size_t类型
```

你也可以对`auto`进行修饰，例如，可以要求模板参数是一个指针：

```cpp
template<const auto* P>
struct S;
```

通过使用可变参数模板，你可以使用一系列不同类型的持久参数作为模板实参：

```cpp
template<auto... VS>
class HeteroValueList {
};
```

或者使用一系列相同类型的模板参数：

```cpp
template<auto V1, decltype(V1)... VS>
class HomoValueList {
};
```

例如:

```cpp
HeteroValueList<1, 2, 3> vals1;         //OK
HeteroValueList<1, 'a', true> vals2;    //OK
HomoValueList<1, 2, 3> vals3;           //OK
HomoValueList<1, 'a', true> vals4;      //ERROR
```

### 12.1.1 字符和字符串的参数化模板

这个特性的一个应用是允许传递字符或字符串作为模板参数。例如，我们可以改进我们可以通过使用如下的折叠表达式来改进我们输出任意数量参数的方法：

*tmpl/printauto.hpp*

```cpp
#include <iostream>

template<auto Swp=' ', typename First, typename... Args>
void print(First first, const Args&... args) {
    std::cout << first;
    auto outWithSpace = [](auto const& arg) {
                            std::cout << Sep << arg;
                        };
    (... , outWithSpace(args));
    std::cout << '\n';
}
```

我们可以使用一个空格作为默认的分隔符来打印出所有的参数：

```cpp
template<auto Sep=' ', typename First, typename... Args>
void print(First firstarg, const Args&... args) {
    ...
}
```

这里，我们可以调用：

```cpp
std::string s{"world"};
print(7.5, "hello", s);     //打印出：7.5 hello world
```

我们可以显式传递一个字符来作为第一个模板参数：

```cpp
print<'-'>(7.5, "hello", s);    //打印出：7.5-hello-world
```

因为使用了`auto`，我们甚至可以传递一个持久的字符串字面量（见第11章），像下面这样：

```cpp
static const char sep[] = ", ";
print<sep>(7.5, "hello", s);    //打印出：7.5, hello, world
```

或者我们可以使用任何其他有效的类型作为模板参数：

```cpp
print<-11>(7.5, "hello", s);    //打印出：7.5-11hello-11world
```

### 12.1.2 定义元编程常量

模板参数的`auto`特性的另一个应用是更容易的定义编译期常量。不需要再像下面这样定义：

```cpp
template<typename T, T v>
struct constant
{
    static constexpr T value = v;
};

using i = constant<int, 42>;
using c = constant<char, 'x'>;
using b = constant<bool, true>;
```

你现在可以像这样做：

```cpp
template<auto v>
struct constant
{
    static constexpr auto value = v;
};

using i = constant<42>;
using c = constant<'x'>;
using b = constant<true>;
```

也不需要再这样写：

```cpp
template<typename T, T... Elements>
struct sequence {
};

using indexes = sequence<int, 0, 3, 4>;
```

而是可以这样实现：

```cpp
template<auto... Elements>
struct sequence {
};

using indexes = sequence<0, 3, 4>;
```

你甚至可以使用一列不同类型的值作为参数：

```cpp
using tuple = sequence<0, 'h', true>;
```

## 12.2 使用`decltype(auto)`作为模板参数

你也可以使用另一个类型占位符，即C++14引入的`decltype(auto)`。然而，请注意这个类型的推导规则非常特殊。根据`decltype`的特性，如果传递的是表达式而不是名字，那么它将从表达式中的值类型体系（见5.3节）推导出表达式类型：

* prvalue(例如临时变量)会推导出type(值类型)
* lvalue(例如命名的对象)会推导出type&(引用类型)
* xvalue(例如使用`std::move()`转换而成的右值引用)会推导出type&&(右值引用类型)

这就意味着，你可以轻易将模板参数推断为引用类型，这将导致令人惊奇的效果。例如：

*tmpl/decltypeauto.cpp*

```cpp
#include <iostream>

template<decltype(auto) N>
struct S {
    void printN() const {
        std::cout << "N: " << N << '\n';
    }
};

static const int c = 42;
static int v = 42;

int main()
{
    S<c> s1;        //N会被推导为const int 42
    S<(c)> s2;      //N会被推导为c的const int&
    s1.printN();
    s2.printN();

    S<(v)> s3;      //N会被推导为v的int&
    v = 77;
    s3.printN();    //打印出：N: 77
}
```

## 12.3 后记

非类型模板参数的类型占位符最早由James Touton和Michael Spertus在[https://wg21.link/n4469](https://wg21.link/n4469)上作为部分内容提出。最终被接受的正式提案由James Touton和Michael Spertus在[https://wg21.link/p0127r2](https://wg21.link/p0127r2)中发表。



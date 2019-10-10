# Chapter 13 扩展的`using`声明

`using`声明被扩展后允许使用逗号分隔一系列声明，这可以用于扩展一个类。

例如，你现在可以这么写：

```cpp
class Base {
  public:
    void a();
    void b();
    void c();
};

class Derived : private Base {
  public:
    using Base::a, Base::b, Base::c;
}
```

## 13.1 使用可变长度的`using`声明

逗号分隔的`using`声明给我们提供了一种简单的方法来继承不定数量的基类中的某一种操作。

这项技术的一个很酷的应用是创建并设置lambda表达式重载。通过像下面这样定义：

*tmpl/overload.hpp*

```cpp
//继承所有基类中的函数调用运算符
template<typename... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};

//基类类型通过传入的参数来推导
template<typename... Ts>
overload(Ts...) -> overload<Ts...>;
```

你可以像如下方式重载两个lambda：

```cpp
auto twice = overload {
    [](std::string& s) { s += s;},
    [](auto& v) { v *= 2;}
};
```

这里我们创建了一个`overload`类型的对象，我们使用了推导指引（见第8章）来推断lambda的类型作为`overload`的基类。我们还使用了聚合体初始化（见第4章）来使用lambda转化而成的闭包类型初始化新的对象。

`using`声明让两个函数调用元素符对类型`overload`而言都是有效的。如果没有`using`声明，因为两个基类都对同一个函数`()`有不同的重载版本，所以会导致歧义。

你可以传递一个字符串，它将调用第一个重载。或者你可以传递另一种类型（能使用*=元素符的类型），这将会调用第二个重载版本：

```cpp
int i = 42;
twice(i);
std::cout << "i: " << i << '\n';    //打印出:84
std::string s = "hi";
twice(s);
std::cout << "s: " << s << '\n';    //打印出:hihi
```

这项技术的另一个应用是`std::variant`的遍历器（见15.2.3节）。

## 13.2 在继承构造函数时使用可变的`using`声明

除了直接声明继承构造函数之外，下面的写法现在也是可行的：你可以声明一个可变类模板Multi，它可以继承所有基类的构造函数：

*tmpl/using2.hpp*

```cpp
template<typename T>
class Base {
    T value{};
  public:
    Base() {
        ...
    }
    Base(T v) : value{v} {
        ...
    }
    ...
};

template<typename... Types>
class Multi : private Base<Types>...
{
  public:
    //继承所有构造函数
    using Base<Types>::Base...;
    ...
}
```

有了所有基类的构造函数的`using`声明，你就继承了每一个基类的构造函数。现在，当使用有三种类型的`Multi<>`类型时：

```cpp
using MultiISB = Multi<int, std::string, bool>;
```

你可以使用每种类型相应的构造函数：

```cpp
MultiISB m1 = 42;
MultiISB m2 = std::string("hello");
MultiISB m3 = true;
```

根据新的语言规则，每一个初始化都会调用相应基类的相应构造函数和所有其他基类的默认构造函数。因此

```cpp
MultiISB m2 = std::string("hello");
```

会调用`Base<int>`的默认构造函数，`Base<std::string>`的字符串构造函数和`Base<bool>`的默认构造函数。

原则上讲，你也可以通过使用：

```cpp
template<typename... Types>
class Multi : private Base<Types>...
{
    ...
    //继承所有赋值运算符
    using Base<Types>::operator=...;
};
```

来允许`Multi<>`进行赋值操作。

## 13.3 后记

逗号分割的`using`声明最早由Robert Haberlach在[https://wg21.link/p0195r0](https://wg21.link/p0195r0)上提出。最终被接受的正式提案由Robert Haberlach和Richard Smith在[https://wg21.link/p0195r2](https://wg21.link/p0195r2)上正式发表。

关于继承构造函数有很多核心的错误被提出。这些错误最终的解决方案由Richard Smith在[https://wg21.link/n4429](https://wg21.link/n4429)上发表。

还有一个Vicente J. Botet Escriba提出的提案是添加一个泛型的`overload`函数，除了能重载lambda之外，还能重载普通函数和成员函数。然而，这个提案并没能进入C++17标准。可以访问[https://wg21.link/p0051r1](https://wg21.link/p0051r1)了解详情。

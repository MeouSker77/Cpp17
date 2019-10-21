# Chapter 20 `type traits`扩展

对于`type traitss`，C++17扩展了使用他们的能力并且引入了一些新的`type traits`。

## 20.1 `type traits`后缀`_v`

自从C++17，你可以使用后缀`_v`来让所有`type traits`返回一个值（同样你可以使用后缀`_t`让一个`type traits`返回一个类型）。例如，对于任意类型`T`以前的

```cpp
std::is_const<T>::value
```

现在可以这么写：

```cpp
std::is_const_v<T>      //自从C++17
```

这对所有的`type traits`都有效。关键是每一个`type trait`现在都有一个对应的变量模板。例如：

```cpp
namespace std {
    template<typename T>
    constexpr bool is_const_v = is_const<T>::value;
}
```

通常这对写`bool`类型的条件很有帮助，你可以在运行时使用它：

```cpp
if (std::is_signed_v<char>) {
}
```

但是因为`type traits`是在编译期求值，因此你可以使用编译期`if`语句来在编译期使用结果（见第9章）：

```cpp
if constexpr (std::is_signed_v<char>) {
    ...
}
```

或者当实例化模板时：

```cpp
//类C的主模板
template<typename T, bool = std::is_pointer_v<T>>
class C {
    ...
};

//指针类型的部分特化模板
template<typename T>
class C<T, true> {
    ...
};
```

这里，类`C`，为指针类型提供了一个特殊版本的实现。

然后后缀`_v`也可被用于返回非`bool`值的`type traits`，例如`std::extent<>`，它会返回一个原生数组的某一维度的大小：

```cpp
int a[5][7];
std::cout << std::extent_v<decltype(a)> << '\n';    //打印出5
std::cout << std::extent_v<decltype(a), 1> << '\n'; //打印出7
```

## 20.2 新的`type traits`

C++17引入了一些新的`type traits`（见20.1节）。另外，自从C++17起`is_literal_type<>`和`result_of<>`被废弃了。

#### `is_aggregate<>`

`std::is_aggergate<T>`判断出`T`是否是一个聚合体类型（见4.3节）：

```cpp
template<typename T>
struct D : std::string, std::complex<T> {
    std::string data;
};

D<float> s{{"hello"}, {4.5, 6.7}, "world"};         //OK自从C++17
std::cout << std::is_aggregate<decltype(s)>::value; //输出：1（true）
```

## 20.3 `std::bool_constant<>`

如果`traits`返回`bool`值，他们现在通常使用模板别名`bool_constant<>`：

```cpp
namespace std {
    template<bool B>
    using bool_constant = integral_constant<bool, B>;   //自从C++17
    using true_type = bool_constant<true>;
    using false_type = bool_constant<false>;
}
```

在C++17之前，`std::true_type`和`std::false_type`是直接特化版的`integral_constant<bool, true>`和`integral_constant<bool, false>`。

当使用特定的类型来实例化一个`bool`类型的`trait`时它通常继承自`std::true_type`，当使用其他类型时将继承自`std::false_type`，例如：

```cpp
//主模板：当泛型类型T不为void类型时
template<typename T>
struct IsVoid : std::false_type {
};

//类型void的特化版：
template<>
struct IsVoid<void> : std::true_type {
};
```

但是现在你可以通过继承`bool_constant<>`来定义你自己的`type trait`，只要你能将相应的`bool`条件写成一个编译器的表达式。例如：

```cpp
template<typename T>
struct IsLargerThanInt : std::bool_constant<(sizeof(T)>sizeof(int))> {
}
```

因此你可以使用这样一个`trait`来根据类型是否大于`int`进行不同的编译：

```cpp
template<typename T>
void foo(T x)
{
    if constexpr(IsLargerThanInt<T>::value) {
        ...
    }
}
```

通过为后缀`_v`添加相应的变量模板（见20.1节）：

```cpp
template<typename T>
static constexpr auto IsLargerThanInt_v = IsLargerThanInt<T>::value;
```

你也可以像下面这样缩短`trait`的使用：

```cpp
template<typename T>
void foo(T x)
{
    if constexpr(IsLargerThanInt_v<T>) {
        ...
    }
}
```

作为另一个例子，我们可以定义一个检查类型`T`是否有一个保证不抛出异常的`move`构造函数的`trait`：

```cpp
template<typename T>
struct IsNothrowMoveConstructibleT : std::bool_constant<noexcept(T(std::declval<T>()))> {
};
```

## 20.4 `std::void_t<>`

一个微小但是在定义`type trait`时候非常有用的东西在C++17中被标准化了：`std::void_t<>`。它有如下的简单定义：

```cpp
namespace std {
    template<typename ...> using void_t = void;
}
```

也就是说，它对于任何可变参数模板类型都返回`void`。当我们想将一个参数类型列表作为一个整体进行处理的时候这会很有用：

```cpp
#include <utility>      //for declval<>
#include <type_trait>   //for true_type, false_type, void_t

//主模板
template<typename, typename = std::void_t<>>
struct HasVarious : std::false_type {
};

//部分特化版(可能因为SFNAE而被忽略)
template<typename T>
struct HasVarious<T, std::void_t<decltype(std::declval<T>().begin()),
                    typename T::difference_type, 
                    typename T::iterator>>
                     : std::true_type {
};
```

这里我们定义了一个新的`type trait`--`HasVarious`，它将检查以下三件事：

* 这个类型有一个成员函数`begin()`吗
* 这个类型有一个类型成员`difference_type`吗
* 这个类型有一个类型成员`iterator`吗

只有当类型`T`同时满足三个相应的条件时才会使用部分特化的版本。它的特化程度比主模板更高，我们可以从`std::true_type`（见20.3节）派生它，我们可以像下面这样使用它：

```cpp
if constexpr (HasVarious<T>::value) {
    ...
}
```

如果任何一个表达式返回无效的结果（例如，`T`没有`begin()`，或者没有类型成员`difference_type`或者没有类型成员`iterator`），根据规则*substitution failure is not an error*这个部分特化版本将会因为SFNAE而被忽略。然后，就只有派生自主模板的`std::false_type`可用（见20.3节），因此就达到了检查的目的。

同样的方法，你可以使用`std::void_t`来轻易的定义其他的`trait`来检查一个或多重条件，这些条件可能是是否有一个成员或操作，或这些成员或操作是否有某些能力。

## 20.5 后记

为可变模板制定标准的`type trait`最早由Stephan T. Lavavej在2014年在[https://wg21.link/n3854](https://wg21.link/n3854)上提出。最终由Alisdair Meredith在[https://wg21.link/p0006r0](https://wg21.link/p0006r0)上第一次提出加入标准库技术规范。

`type trait` `std::is_aggregate<>`最早在美国国家机构对C++17标准的注释中引入（见[https://wg21.link/lwg2911](https://wg21.link/lwg2911)）。

`std::bool_constant<>`最早由Zhihao Yuan在[https://wg21.link/n4334](https://wg21.link/n4334)上提出。最后被接受的正式提案由Zhihao Yuan在[https://wg21.link/n4389](https://wg21.link/n4389)上发表。

`std::void_t<>`被接受的提案由Walter E. Brown在[https://wg21.link/n3911](https://wg21.link/n3911)上提出。

**正在完善**
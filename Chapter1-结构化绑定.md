# Chapter 1 结构化绑定

结构化绑定允许你用一个对象的成员或元素初始化多个变量。例如，如果你定义了一个有两个不同成员的结构体：

```cpp
struct MyStruct {
    int i = 0;
    std::string s;
};

MyStruct ms;
```

你可以通过如下声明用两个变量直接绑定该结构体的两个成员：

```cpp
auto [u,v] = ms;
```

在这里，`u`和`v`被称为结构化绑定。某种程度上它们通过分解对象来初始化（某种意义上它们也被成为分解声明式）。

结构化绑定对于返回结构体或数组的函数尤其有用。例如，如果你有一个返回一个结构体的函数：

```cpp
MyStruct getStruct() {
    return MyStruct{42, "hello"};
}
```

你可以直接把返回值的两个数据成员赋值给两个局部变量：

```cpp
auto [id,val] = getStruct(); //id和val分别对应返回结构体的i和s成员
```

在这里，`id`和`val`分别是返回结构体中名为`i`和`s`的变量。它们有不同的类型，即`int`和`std::string`。它们可以被当作两个不同的变量使用：

```cpp
if (id > 30) {
    std::cout << val;
}
```

这种写法的一个好处是可以直接访问变量，另外，我们还可以将值绑定到能体现语义的变量名上，以增强代码的可读性。

接下来的代码显示了结构化绑定如何显著地改进代码。在没有结构化绑定的情况下，为了迭代一个`std::map<>`的元素你必须像下边这样编码：

```cpp
for (const auto& elem : mymap) {
    std::cout << elem.first << ": " << elem.second << '\n';
}
```

`elem`元素的类型是`std::pair`，你需要用它的`first`和`second`成员访问键和值。通过使用结构化绑定，可以让代码的可读性更强：

```cpp
for (const auto& [key,val] : mymap) {
    std::cout << key << ": " << val << '\n';
}
```

通过这种方式，我们可以直接用简单的变量名来访问每个元素的键和值。

## 1.1 结构化绑定的细节

为了理解结构化绑定，很重要的一点是要认识到其中有一个匿名变量参与。结构化绑定中新引入的变量名指向匿名变量。

#### 绑定到匿名实体

如下初始化语句的精确行为

```cpp
auto [u,v] = ms;
```

就像是我们用`ms`的值初始化了一个新的实体`e`，然后用结构化绑定中的`u`和`v`作为这个新实体的成员的别名，类似于如下定义

```cpp
auto e = ms;
auto& u = e.i;
auto& v = e.s;
```

唯一的不同之处在于我们没有赋给`e`一个变量名，所以我们不能用变量名直接访问这个匿名实体。如下语句的结果

```cpp
std::cout << u << ' ' << v << '\n';
```

会打印出`e.i`和`e.s`的值，这两个值是`ms.i`和`ms.s`的拷贝。

只要指向`e`的结构化绑定存在，`e`就会继续存在。因此，当结构化绑定离开作用域时，`e`也会被销毁。

修改结构化绑定中用于初始化的对象的值并不会改变结构化绑定的变量值（反之亦然）：

```cpp
MyStruct ms{42, "hello"};
auto [u,v] = ms;
ms.i = 77;
std::cout << u;     //打印出42
u = 99;
std::cout << ms.i;  //打印出77
```

`u`和`ms.i`拥有不同的内存地址。

当对函数返回值使用结构化绑定时相同的原则依然适用。如下的一个初始化

```cpp
auto [u,v] = getStruct();
```

其行为类似于我们用`getStruct()`的返回值初始化了一个新的实体`e`，然后我们使用结构化绑定将`u`和`v`绑定为`e`的两个成员的别名，类似于如下定义：

```cpp
auto e = getStruct();
auto& u = e.i;
auto& v = e.s;
```

也就是说，结构化绑定绑定到了一个用返回值初始化的新的实体，而不是直接绑定到返回值上。

地址和内存对齐的规则对匿名实体`e`来说同样适用，因此结构化绑定的变量会像它们绑定到的成员一样对齐。例如：

```cpp
auto [u,v] = ms;
assert(&((MyStruct*)&u)->s == &v);  //OK
```

在这里，`((MyStruct*)&u)`可以得到一个指向匿名实体的指针。

#### 使用修饰词

我们可以使用修饰词，例如`const`和引用。这些修饰词会应用在整个匿名实体`e`上。通常，这么写的效果类似于直接对结构化绑定应用这些修饰词，但请注意并不总是这样（看下面的例子）。

例如，我们可以声明一个对常量引用的结构化绑定：

```cpp
const auto& [u,v] = ms; //一个引用，因此u/v指向ms.i/ms.s
```

在这里，匿名实体被声明为一个常量的引用，这意味着`u`和`v`将是`ms`中成员`i`和`s`的常量引用。因此，任何对`ms`成员的修改都会影响`u`和`v`的值。

```cpp
ms.i = 77;          //影响u的值
std::cout << u;     //打印出77
```

如果声明一个非常量引用，你甚至可以修改原对象成员的值：

```cpp
MyStruct ms{42, "hello"};
auto& [u,v] = ms;           //初始化的匿名实体是ms的一个引用
ms.i = 77;                  //会影响u的值
std::cout << u;             //打印出77
u = 99;                     //会修改ms.i
std::cout << ms.i;          //打印出99
```

如果用于结构化绑定引用的值是一个临时对象，该临时对象的生存周期将延长到结构化绑定的生存周期：

```cpp
MyStruct getStruct();
...
const auto& [a,b] = getStruct();
std::cout << "a: " << a << '\n';    //OK
```

#### 修饰词未必会应用到结构化绑定上

就像标题说的那样，修饰词会应用到新的匿名实体上，而不是应用到结构化绑定中引入的新变量名上。这两者的不同可以通过指明内存对齐来体现：

```cpp
alignas(16) auto [u,v] = ms;    //对齐匿名对象，而不是v
```

这里，我们对齐了初始化的匿名对象而不是结构化绑定中的`u`和`v`。这意味着`u`作为匿名对象的第一个成员会进行16字节对齐，但`v`不会。

出于同样的原因，即便使用了`auto`关键字，结构化绑定也不会发生类型退化（这里术语**退化**指的是参数以值传递时发生的类型转换，比如原生数组会转换为指针，比如`auto`推断类型时会忽略`const`和引用修饰词）。例如，如果我们有一个有几个原生数组的结构体：

```cpp
struct S {
    const char x[6];
    const char y[3];
};
```

然后

```cpp
S s1{};
auto [a,b] = s1;    //a和b会得到成员的精确类型
```

`a`的类型仍然是`const char [6]`。`auto`会应用于匿名实体，该实体作为一个整体被初始化，因此内部成员是不会发生类型退化的。这和直接用`auto`初始化一个新的对象是不同的，后者会发生类型退化：

```cpp
auto a2 = a;    //a2会得到a退化后的类型，即const char *
```

#### Move 语义

Move语义也支持刚才所述的规则，看下面的声明：

```cpp
MyStruct ms = {42, "Jim"};
auto&& [v,n] = std::move(ms);   //匿名实体是ms的右值引用
```

此处结构化绑定的`v`和`n`指向的匿名实体是`ms`的一个右值引用。`ms`仍然保持它的值：

```cpp
std::cout << "ms.s: " << ms.s << '\n';  //打印出"Jim"
```

但是你可以用move赋值搬移`n`的值：

```cpp
std::string s = std::move(n);           //将ms.s搬移到s
std::cout << "ms.s: " << ms.s << '\n';  //打印出不确定的值
std::cout << "n:    " << n << '\n';     //打印出不确定的值
std::cout << "s:    " << s << '\n';     //打印出"Jim"
```

像通常一样，被搬移的对象处于一种有效但值不确定的状态。因此，打印它的值是没问题的，但不要对打印出的内容做任何假设。

这和直接使用`ms`被move的值来初始化新的匿名实体有一些不同：

```cpp
MyStruct ms {42， "Jim"};
auto [v,n] = std::move(ms);     //新实体的值从ms中move而来
```

在这里，匿名实体是一个用`ms`被move的值初始化的新对象。因此，`ms`已经失去了自己的值：

```cpp
std::cout << "ms.s: " << ms.s << '\n';  //打印出不确定的值
std::cout << "n:    " << n  << '\n';    //打印出"Jim"
```

你可以继续搬移`n`的值或者赋予`n`一个新值，但这并不影响`ms.s`：

```cpp
std::string s = std::move(n);           //把n的值move到s
n = "Lara";
std::cout << "ms.s: " << ms.s << '\n';  //打印出不确定的值
std::cout << "n:    " << n << '\n';     //打印出"Lara"
std::cout << "s:    " << s << '\n';     //打印出"Jim"
```

## 1.2 结构化绑定的应用场景

原则上讲，结构化绑定可以被用于有`public`数据成员的结构体、原生C风格的数组、和类似元组的对象：

* 如果在一个**struct或class**中所有非静态数据成员都是`public`的，你可以用新的名称绑定其中的每一个非静态数据成员。
* 对于**原生数组**，你可以用新的名称绑定每一个元素
* 对于任何提供**元组API**的类型你都可以绑定其中的元素。对于一个名为`type`的类型而言，要想满足该API需要满足以下几点：

    \- `std::tuple_size<type>::value`应该返回元素的个数

    \- `std::tuple_element<idx,type>::type`应该返回索引为`idx`的元素的类型

    \- 一个全局或成员函数`get<idx>()`应该返回索引为`idx`的元素的值（ 果该索引处有元素）

标准库类型中的`std::pair<>`, `std::tuple<>`, `std::array<>`已经提供了该API。

无论在何种情况下，结构化绑定声明中的变量数量都应该和元素或数据成员的数量保持一致。你不可以忽略某些变量名也不可以使用同一个变量名两次。然而，你可以用一个非常短的名字例如'_'，但一个名字在同一个作用域中只能用一次：

```cpp
auto [_,val1] = getStruct();    //OK
auto [_,val2] = getStruct();    //错误：变量名_已经被使用过
```

结构化绑定不支持嵌套声明或多维声明。

接下来的子章节详细讨论这些情况。

### 1.2.1 Structs 和 Classes

以上的示例都只是展示了一些简单的`struct`和`class`的结构化绑定，注意那些这只适用于继承受限制的情况下。对`struct`和`class`使用结构化绑定时，所有非静态的数据成员必须在同一个类定义中（因此，只有当所有非静态数据成员都处于基类或都处于派生类中时才能使用结构化绑定）：

```cpp
struct B {
    int a = 1;
    int b = 2;
};

struct D1 : B {
};
auto [x, y] = D1{};     //OK

struct D2 : B {
    int c = 3;
};
auto [i, j, k] = D2{};  //编译期错误
```

### 1.2.2 原生数组

下面的代码用c风格数组的两个元素初始化了`x`和`y`：

```cpp
int arr[] = {47, 11};
auto [x, y] = arr;      //x和y用arr中的元素初始化
auto [z] = arr;         //错误：元素数量和变量数量不匹配
```

只要数组长度已知结构化绑定就是可行的，然而当数组被当作参数传入时，就会变得不可行，因为数组会被退化为指针类型。

注意C++允许我们返回一个带有长度信息的数组，因此绑定的特性也适用于返回带长度数组的函数：

```cpp
auto getArr() -> int(&)[2]; //getArr()返回一个原生int类型数组的引用
...
auto [x, y] = getArr();      //x和y被返回的数组中的元素初始化
```

你也可以对`std::array`使用结构化绑定，不过这是用元组API的方式实现的，接下来对这种方式进行描述。

### 1.2.3 `std::pair`, `std::tuple`, 和`std::array`

结构化绑定机制是可扩展的，因此你可以为任何类型添加对结构化绑定的支持。标准库里为`std::pair<>`, `std::tuple<>`, `std::array<>`添加了支持。

#### `std::array`

例如，下面的代码用`std::array<>`中的四个元素初始化了`i`，`j`，`k`，`l`：

```cpp
std::array<int, 4> getArray();
...
auto [i,j,k,l] = getArray();    //i,j,k,l绑定了返回的数组中的四个元素
```

这里，`i`，`j`，`k`，`l`被绑定到返回的`std::array`中的元素。

只要用于初始化的值不是临时的返回值，那么就可以通过引用进行修改。例如：

```cpp
std::array<int, 4> stdarr {1, 2, 3, 4};
...
auto& [i,j,k,l] = stdarr;
i += 10;    //修改了stdarr[0]
```

#### `std::tuple`

下面的代码用`getTuple()`函数返回的`std::tuple<>`中的元素初始化了`a`，`b`，`c`：

```cpp
std::tuple<char, float, std::string> getTuple();
...
auto [a,b,c] = getTuple();  //a,b,c绑定了返回的tuple中的元素
```

在这里，`a`的类型为`char`，`b`的类型为`float`，`c`的类型为`std::string`。

#### `std::pair`

作为另一个例子，下面代码将关联式容器的`insert()`函数的返回值绑定到两个能体现语义的变量名上，而不是用`std::pair<>`的`first`和`second`成员进行访问：

```cpp
std::map<std::string, int> coll;
...
auto [pos, ok] = coll.insert({"new", 42});
if (!ok) {
    //如果插入失败，就用迭代器pos处理错误
    ...
}
```

在C++17之前，要想实现同样的效果就要像下面这样写：

```cpp
auto ret = coll.insert({"new", 42});
if (!ret.second) {
    //如果插入失败，就用迭代器ret.first处理错误
}
```

注意在这种特殊情况下，C++17提供了一种带初始化语句的`if`表达式来进行改进（见2.1节）。

## 1.3 为结构化绑定提供元组API

你可以通过提供元组API来为任何类型添加结构化绑定的支持，就像标准库为`std::pair<>`, `std::tuple<>`和`std::array<>`所做的那样。

#### 允许只读结构化绑定

下面的例子展示了如何为一个自定义的类`Customer`添加只读结构化绑定的支持：该类的定义可能如下：

*lang/customer1.hpp*

```cpp
#include <string>
#include <utility>   //for std::move()

class Customer {
private:
    std::string first;
    std::string last;
    long val;
public:
    Customer(std::string f, std::string l, long v)
    : first(std::move(f)), last(std::move(l)), val(v) {
    }
    std::string getFirst() const {
        return first;
    }
    std::string getLast() const {
        return last;
    }
    long getValue() const {
        return val;
    }
};
```

我们可以像下面这样提供元组API：

*lang/structbind1.hpp*

```cpp
#include "customer1.hpp"
#include <utility>  //for 元组API

//为Customer类提供元组API来支持结构化绑定
template<>
struct std::tuple_size<Customer> {
    static constexpr int value = 3;  //Customer类有三个元素
};

template<>
struct std::tuple_element<2, Customer> {
    using type = long;      //最后一个元素的类型是long
};

template<std::size_t Idx>
struct std::tuple_element<Idx, Customer> {
    using type = std::string;   //其他的元素类型是std::string
};

//定义特定的get函数
template<std::size_t> auto get(const Customer& c);
template<> auto get<0>(const Customer& c) {return c.getFirst();}
template<> auto get<1>(const Customer& c) {return c.getLast();}
template<> auto get<2>(const Customer& c) {return c.getValue();}
```

这里，我们为自定义的顾客类型定义了元组API，将顾客的3个属性映射到了不同的`getter`(其他自定义的映射也是可行的)：

* 顾客的姓是`std::string`类型
* 顾客的名是`std::string`类型
* 顾客的消费金额是`long`类型

将`Customer`类的成员数量定义为`std::tuple_size`模板对`Customer`类的特化：

```cpp
template<>
struct std::tuple_size<Customer> {
    static const int value = 3;     //Customer类有三个属性
};
```

3个成员的类型被定义为`std::tuple_element`模板的特化：

```cpp
template<>
struct std::tuple_element<2, Customer> {
    using type = long;      //最后一个属性的类型是long
};
template<std::size_t Idx>
struct std::tuple_element<Idx, Customer> {
    using type = std::string;   //其他的属性类型为std::string
};
```

其中第三个属性的类型被定义为`Idx`为2时的全特化版本。对于其他属性则使用半特化版本，函数匹配时半特化版本的优先级要低于全特化版本。

最后，我们定义了相应的`getter`作为函数`get<>()`对`Customer`类型的重载:

```cpp
template<std::size_t> auto get(const Customer& c);
template<> auto get<0>(const Customer& c) {return c.getFirst();}
template<> auto get<1>(const Customer& c) {return c.getLast();}
template<> auto get<2>(const Customer& c) {return c.getValue();}
```

在这个例子中，我们首先定义了`get<>()`模板函数对`Customer`类型的重载，然后我们为重载的模板定义了每一种情况的全特化函数。注意所有的全特化函数模板必须有完全相同的签名（包括返回值）。原因是我们是要提供特化版的实现，而不是要提供新的声明。下面语句不能通过编译：

```cpp
template<std::size_t> auto get(const Customer& c);
template<> std::string get<0>(const Customer& c) {return c.getFirst();}
template<> std::string get<1>(const Customer& c) {return c.getLast();}
template<> long get<2>(const Customer& c) {return c.getValue();}
```

通过使用新的编译期`if`语句特性（见第9章），我们可以将`get<>()`模板的实现整合到一个函数中。

```cpp
template<std::size_t I> auto get(const Customer& c) {
    static_assert(I < 3);
    if constexpr (I == 0) {
        return c.getFirst();
    }
    else if constexpr (I ==1) {
        return c.getLast();
    }
    else { //如果I == 2
        return c.getValue();
    }
}
```

有了这个API，我们就可以为`Customer`类型使用结构化绑定：

*lang/structbind1.cpp*

```cpp
#include "structbind1.hpp"
#include <iostream>

int main()
{
    Customer c("Tim", "Starr", 42);
    auto [f, l, v] = c;
    std::cout << "f/l/v:    " << f << ' ' << l << ' ' << v << '\n';

    //修改绑定的变量
    std::string s = std::move(f);
    l = "Waters";
    v += 10；
    std::cout << "f/l/v:    " << f << ' ' << l << ' ' << v << '\n';
    std::cout << "c:        " << c.getFirst() << ' '
              << c.getLast() << ' ' << c.getValue() << '\n';
    std::cout << "s:        " << s << '\n';
}
```

和往常一样，结构化绑定先用`c`初始化一个匿名实体，再将`f`,`l`,`v`定义为匿名实体的成员的引用。这个过程中，初始化语句会调用每一个成员对应的`getter`。因此，初始化完成之后修改`c`不会对绑定的变量造成任何影响（反之亦然）。因此，程序会有如下输出：

```
    f/l/v:  Tim Starr 42
    f/l/v:   Waters 52
    c:      Tim Starr 42
    s:      Tim
```

#### 允许可写的结构化绑定

元组API可以使用返回引用的函数，这样能让结构化绑定拥有写变量的能力。考虑为`Customer`类提供API来读写它的成员：

*lang/customer2.hpp*

```cpp
#include<string>
#include<utility> //for std::move()

class Customer {
private:
    std::string first;
    std::string last;
    long val;
public:
    Customer(std::string f, std::string l, long v)
    : first(std::move(f)), last(std::move(l)), val(v) {
    }
    const std::string& firstname() const {
        return first;
    }
    std::string& firstname() {
        return first;
    }
    const std::string& lastname() const {
        return last;
    }
    std::string& lastname() {
        return last;
    }
    long value() const {
        return val;
    }
    long& value() {
        return val;
    }
};
```

为了获得读写权限，我们需要重载`getter`的常量和非常量引用的版本：

*lang/structbind2.hpp*

```cpp
#include "customer2.hpp"
#include <utility>  //for 元组API

//为Customer类提供结构化绑定所需的元组API:
template<>
struct std::tuple_size<Customer> {
    static constexpr int value = 3;     //Customer类有三个成员
};

template<>
struct std::tuple_element<2, Customer> {
    using type = long;  //最后一个成员的类型是long
};
template<std::size_t Idx>
struct std::tuple_element<Idx, Customer> {
    using type = std::string;   //其他的成员类型是string
};

//定义特化的getters:
template<std::size_t I> decltype(auto) get(Customer& c) {
    static_assert(I < 3);
    if constexpr (I == 0) {
        return c.firstname();
    }
    else if constexpr (I == 1) {
        return c.lastname();
    }
    else {  //I == 2
        return c.value();
    }
}
template<std::size_t I> decltype(auto) get(const Customer& c) {
    static_assert(I < 3);
    if constexpr (I == 0) {
        return c.firstname();
    }
    else if constexpr (I == 1) {
        return c.lastname();
    }
    else {  //I == 2
        return c.value();
    }
}
template<std::size_t I> decltype(auto) get(Customer&& c) {
    static_assert(I < 3);
    if constexpr (I == 0) {
        return std::move(c.firstname());
    }
    else if constexpr (I == 1) {
        return std::move(c.lastname());
    }
    else {  //I == 2
        return c.value();
    }
}
```

注意你应该有这三份重载来分别处理常量，非常量和可搬移的对象。为了让返回值是一个引用，你需要使用`decltype(auto)`（`decltype(auto)`从C++14引入，当使用`auto`进行类型推断时会发生退化，即会忽略引用修饰词，`decltype(auto)`用来解决这个问题）.

我们又一次使用了新的编译期`if`特性（见第9章），这会简化我们不同版本`getter`的实现。如果没有它，我们就要再次写出完整的全特化版本，例如：

```cpp
template<std::size_t> decltype(auto) get(Customer& c);
template<> decltype(auto) get<0>(Customer& c) { return c.firstname(); }
template<> decltype(auto) get<1>(Customer& c) { return c.lastname(); }
template<> decltype(auto) get<2>(Customer& c) { return c.value(); }
...
```

再一次提醒函数模板的主声明和全特化版本必须有完全相同的签名（包括返回值）。下边的语句将不能通过编译：

```cpp
template<std::size_t> decltype(auto) get(Customer& c);
template<> std::string& get<0>(Customer& c) { return c.firstname(); }
template<> std::string& get<1>(Customer& c) { return c.lastname(); }
template<> long& get<2>(Customer& c) { return c.value(); }
```

现在，你可以使用结构化绑定来获取读写成员的能力了：

*lang/structbind2.cpp*

```cpp
#include "structbind2.hpp"
#include <iostream>

int main()
{
    Customer c("Tim", "Starr", 42);
    auto [f, l, v] = c;
    std::cout << "f/l/v:    " << f << ' ' << l << ' ' << v << '\n';
    auto&& [f2, l2, v2] = c;
    std::string s = std::move(f2);
    l2 = "Waters";
    v2 += 10;
    std::cout << "f2/l2/v2: " << f2 << ' ' << l2 << ' ' << v2 << '\n';
    std::cout << "c:        " << c.firstname() << ' '
              << c.lastname() << ' ' << c.value() << '\n';
    std::cout << "s:        " << s << '\n';
}
```

程序的输出如下：

```
    f/l/v:      Tim Starr 42
    f2/l2/v2:   Tim Starr 42
    c:           Waters 52
    s:          Tim
```

## 1.4 后记

结构化绑定最先被Herb Sutter, Bjarne Stroustrup和Gabriel Dos Reis在[https://wg21.link/p0144r0](https://wg21.link/p0144r0)上提出，当时提议用花括号而不是方括号来表示。该特性最终被接受的正式提案由Jens Maurer在[https://wg21.link/p0217r3](https://wg21.link/p0217r3)发表。

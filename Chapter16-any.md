# Chapter 16 `std::any`

一般来说，C++是一个有类型绑定和类型安全的语言。一个变量声明时就需要一个确切的类型，这个类型声明决定了这个变量可能的行为。而且这个变量不能改变自己的类型。

`std::any`是一个可以改变自己类型的类型，但它仍然具有类型安全特性。也就是说，它可以持有任意类型的值但是它知道当前所持有的值类型。当声明一个这种类型的对象时没有必要指出可能的类型。

它的原理是它同时含有值和这个值的类型，值的类型通过`typeid`来记录。因为这种对象可能需要任意大小的内存空间所以可能会需要在堆上分配内存。然而，实现者应该尽量避免为小类型使用堆内存，例如`int`。

也就是说，如果你用一个字符串给这个对象赋值，这个对象会分配所需内存并拷贝字符串，并同时在内部记录下当前类型是一个字符串。之后，运行时检查可以查明当前值的具体类型并使用`any_cast<>`将这个值转换为它的类型来使用这个值。

和`std::optional<>`，`std::variant<>`一样该对象有值语义。也就是说，拷贝时会发生深拷贝，新的对象会在自己的内存中创建新的内含的值和类型。因为可能会用到堆内存，因此拷贝`std::any`的开销通常很昂贵，你应该尽量以引用传递对象或者搬移对象。该类型支持一部分move语义。

## 16.1 使用`std::any`

下面的代码展示了`std::any`的核心能力：

```cpp
std::any a;             //a是空的
std::any b = 4.3        //b有一个double类型的值，值为4.3
a = 42;                 //a有一个int类型的值，值为42
b = std::string{"hi"};  //b有一个std::string类型的值，值为"hi"

if (a.type() == typeid(std::string)) {
    std::string s = std::any_cast<std::string>(a);
    useString(s);
}
else if (a.type() == typeid(int)) {
    useInt(std::any_cast<int>(a));
}
```

你可以声明一个`std::any`为空，也可以用一个确定类型的值来初始化它。用于初始化的值的类型将作为内含值的类型。

通过使用成员函数`type()`你可以检查内含值的类型ID。如果对象是空的，类型ID将是`typeid(void)`。

为了访问内含的值你必须使用`std::any_cast<>`来转换它的类型：

```cpp
auto s = std::any_cast<std::string>(a);
```

如果转换失败，可能是因为对象为空或者转换的类型不匹配，将会抛出一个`std::bad_any_cast`异常。因此，在不知道值的类型的情况下，你最好像下面这样写：

```cpp
try {
    auto s = std::any_cast<std::string>(a);
    ...
}
catch (std::bad_any_cast& e) {
    std::cerr << "EXCEPTION: " << e.what() << '\n';
}
```

注意`std::any_cast<>`创建了一个传入类型的对象。如果你传递一个`std::string`作为`std::any_cast<>`的模板参数，它将会创建一个临时字符串（一个prvalue），然后使用临时字符串初始化新的对象s。如果不想要这个初始化操作，更好的方法是转换为引用类型来避免创建临时变量：

```cpp
std::cout << std::any_cast<const std::string&>(a);
```

为了能修改值，你需要转换为相应的引用类型：

```cpp
std::any_cast<std::string&>(a) = "world";
```

你可以对`std::any`对象的地址使用`std::any_cast`转换。在这种情况下，如果类型匹配这个转换将返回一个相应的指针类型，否则将返回`nullptr`：

```cpp
auto p = std::any_cast<std::string>(&a);
if (p) {
    ...
}
```

为了清空一个已经存在的`std::any`对象你可以调用：

```cpp
a.reset();          //将对象置为空
```

或者：

```cpp
a = std::any{};
```

或者：

```cpp
a = {};
```

你可以直接检查对象是否为空：

```cpp
if (a.has_value()) {
    ...
}
```

注意值也是以退化后的类型存储的（数组转换为指针，顶层的引用和`const`被忽略）。对于字符串字面量来说值的类型将是`const char *`。为了使用`type()`检查和使用`std::ang_cast<>`转换你必须精确的指明类型：

```cpp
std::any a = "hello";       //type()结果是const char*
if (a.type() == typeid(const char*)) {          //true
    ...
}
if (a.type() == typeid(std::string)) {          //false
    ...
}
std::cout << std::any_cast<const char*>(v[1]) << '\n';  //OK
std::cout << std::any_cast<std::string>(v[1]) << '\n';  //抛出异常
```

这就是几乎全部的操作了。它没有比较运算符（因此，你不能比较或者对象），没有哈希函数，也没有`value()`成员函数。而且因为它的真实类型只有在运行时才能得知，所以也不能使用依据不同类型来进行不同处理的泛型lambda。为了处理当前的值你必须总是使用运行时的转换`std::any_cast<>`，这意味着当进行处理时你必须先判断出它的具体类型。

然而，将`std::any`放入一个容器中也是可行的。例如：

```cpp
std::vector<std::any> v;

v.push_back(42);
std::string s = "hello";
v.push_back(s);

for (const auto& a : v) {
    if (a.type() == typeid(std::string)) {
        std::cout << "string: " << std::any_cast<const std::string&>(a) << '\n';
    }
    else if (a.type() == typeid(int)) {
        std::cout << "int: " << std::any_cast<int>(a) << '\n';
    }
}
```

## 16.2 `std::any`类型和操作

这一小节详细介绍`std::any`的类型和操作

### 16.2.1 `any`类型

在头文件`<any>`中C++标准库定义了如下的`std::any`类：

```cpp
namespace std {
    class any;
}
```

也就是说，`std::any`根本不是模板类。

另外，还定义了下面的类型和对象：

* 异常类`std::bad_any_cast`派生自`std::bad_cast`，后者又派生自`std::exception`，该异常用于转换失败时。

`any`对象还使用了定义在`<utility>`中的对象`std::in_place_type`（类型为`std::in_place_type_t`)。

### 16.2.2 `any`操作

表16.1列出了`std::any`提供的所有操作。

![表16.2](images/16.1.png)

#### 构造

默认情况下，一个`std::any`会初始化为空。

```cpp
std::any a1;    //a1为空
```

如果有一个值用来初始化，它退化后的类型将作为内含值的类型：

```cpp
std::any a2 = 42;       //a2内含值的类型为int
std::any a3 = "hello";  //a2内含值的类型为const char*
```

为了持有一个和用于初始化的值不同的类型，你需要使用`in_place_type`标签：

```cpp
std::any a4{std::in_place_type<long>, 42};
std::any a5{std::in_place_type<std::string>, "hello"};
```

甚至传递给`in_place_type`的类型也会发生退化。下面的声明将会持有一个`const char*`:

```cpp
std::any a5b{std::in_place_type<const char[6]>, "hello"};
```

如果你想用多个值来初始化一个`any`对象，当内含值类型无法从参数类型推导出来时你必须事先创建对象或者加上`std::in_place_type`作为第一个参数：

```cpp
std::any a6{std::complex{3.0, 4.0}};
std::any a7{std::in_place_type<std::complex<double>>, 3.0, 4.0};
```

你甚至可以通过附加的参数传递一个初值列：

```cpp
//用一个以lambda为排序准则的set初始化一个std::any
auto sc = [] (int x, int y) {
            return std::abs(x) < std::abs(y);
        };
std::any a8{std::in_place_type<std::set<int, decltype(sc)>>,            {4, 8, -7, -2, 0, 5},
            sc};
```

注意这里有一个便捷函数`make_any<>()`，它可以接受一个或多个参数（不需要使用`in_place_type`参数）。你必须显式的声明初始值的类型（即使只有一个参数传入它也不会进行推导）：

```cpp
auto a10 = std::make_any<float>(3.0);
auto a11 = std::make_any<std::string>("hello");
auto a13 = std::make_any<std::complex<double>>(3.0, 4.0);
auto a14 = std::make_any<std::set<int, decltype(sc)>>({4, 8, -7, -2, 0, 5}, sc);
```

#### 修改值

它有相应的赋值运算符和`emplace()`操作：

```cpp
std::any a;
a = 42;             //a含有一个int类型的值
a = "hello";        //a含有一个const char*类型的值

a.emplace{std::in_place_type<std::string>, "hello"};    //a含有一个std::string类型的值
a.emplace{std::in_place_type<std::complex<double>>, 4.4, 5.5};      //a含有一个std::complex<double>类型的值
```

#### 访问值

为了访问内含的值你必须使用`std::any_cast<>`转换它的类型。为了将值转换为string你有这样几种做法：

```cpp
std::any_cast<std::string>(a);      //返回值的拷贝

std::any_cast<std::string&>(a);     //通过引用可以修改值

std::any_cast<const std::string&>(a);   //只有读权限的引用
```

这里，如果转换失败，将会抛出一个`std::bad_any_cast`异常。

如果在忽略掉顶层引用和`const`之后内含值和指定类型含有相同的类型ID那么类型转换就是合适的。

你可以传递一个地址来保证当转换失败时会得到一个`nullptr`：

```cpp
std::any_cast<std::string>(&a);         //有写权限的指针

std::any_cast<const std::string>(&a);   //有读权限的指针
```

注意在这里如果想转换为引用将会抛出一个运行时异常：

```cpp
std::any_cast<std::string&>(&a);        //运行时异常
```

#### move语义

`std::any`也支持move语义。然而，注意它的move语义只支持也有拷贝语义的类型。也就是说，**只有move语义的类型**不支持作为内含的值类型。

处理move语义的最佳方法可能不那么明显。因此，这里展示了你应该怎么做：

```cpp
std::string s("hello, world!");

std::any a;
a = std::move(s);           //将s搬移到a

s = std::move(std::any_cast<string&>(a));   //将a作为一个字符串搬移到s
```

像往常的被搬移过值的类型一样，在执行完上述最后一条语句后`a`内含的值将变为不确定的状态。当然，只要你不对`a`的值做任何假设你依然可以将a作为一个字符串使用。

注意：

```cpp
s = std::any_cast<string>(std::move(a));
```

也能生效，但是需要一个额外的`move`。然而，下面的写法是很危险的（尽管它是c++标准里的是一个示例）：

```cpp
std::any_cast<string&>(a) = std::move(s2);      //a将持有一个string
```

这只有在`a`内含值的类型已经为`string`是才能生效。否则，此处转换将会抛出一个`std::bad_any_cast`异常。

## 16.3 后记

`any`对象最早由Kevlin Henney和Beman Dawes于2006年在[https://wg21.link/n1939](https://wg21.link/n1939)上提出，以`Boost.Any`作为参考实现。Beman Dawes，Kevlin Henney和Daniel Krugler在[https://wg21.link/n3804](https://wg21.link/n3804)上提出将它接纳进标准库的技术规范里。

Beman Dawes和Alisdair Meredith在[https://wg21.link/p0220r1](https://wg21.link/p0220r1)上提议将它和其他组件一起加入C++17标准。

Vicente J. Botet Escriba在[https://wg21.link/p0032r3](https://wg21.link/p0032r3)上协调了它和`std::variant<>`，`std::optional<>`的API。Jonathan Wakely在[https://wg21.link/p0504r0](https://wg21.link/p0504r0)上修复了`in_place`标签的行为。

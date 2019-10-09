# Chapter 7 其他语言特性

这一章将会描述一些C++语言核心的一些小的改进。

## 7.1 嵌套命名空间

自从2003年首次提出，一直到现在C++标准委员会终于接受了像下面这样定义嵌套命名空间：

```cpp
namespace A::B::C {
    ...
}
```

等价于下面的：

```cpp
namespace A {
    namespace B {
        namespace C {
            ...
        }
    }
}
```

注意还没有对嵌套的`inline`命名空间的支持。这是因为它无法表明`inline`作用于最内层命名空间还是整个命名空间（两者写法等价且都是有效的）

## 7.2 有定义的表达式求值顺序

许多C++书籍上的代码在直觉性的假设的基础上看起来都是正确的，但严格意义上讲它们可能有未定义的行为。一个例子是多重查找并替换一个字符串中的子字符串：

```cpp
std::string s = "I heard it even works if you don't believe";
s.replace(0,8,"").replace(s.find("even"),4,"sometimes")
                 .replace(s.find("you don't"), 9, "I");
```

通常的设想会认为这段代码是有效的，它会把前八个字符替换为空，把"even"替换为"sometimes"，把"you don't"替换成"I"，因此会得到：

```cpp
    it sometimes works if I believe
```

然而，在C++17之前，最终的结果是没有任何保证的。因为`find()`函数调用可能在需要他们的返回值之前的任意时刻调用，而不是像直觉中的那样从左向右按顺序执行表达式。事实上，所有`find()`函数调用，完全有可能在第一次替换之前就全部执行。如下的结果都是有可能的：

```cpp
    it sometimes workIdon’t believe
    it even worsometiIdon’t believe
    it even worsometimesf youIlieve
```

作为另一个例子，考虑如下打印彼此依赖的几个函数的返回值：

```cpp
std::cout << f() << g() << h();
```

通常的假设会认为先调用`f()`，再调用`g()`，再调用`h()`。然而，这个假设是错误的。`f()`，`g()`和`h()`可能以任意顺序被调用，当这几个函数彼此依赖的时候就可能产生出乎意料的结果。

还有另一个具体的例子，在C++17之前下面的表达式会有未定义的行为：

```cpp
i = 0;
std::cout << ++i << ' ' << --i << '\n';
```

在C++17之前，它可能打印出1 0，也可能打印出0 -1或者0 0。无论`i`是`int`或者用户定义类型结果都是不确定的（如果是基础类型，某些编译器可能会给出警告）。

为了修复这些未定义的行为，一些操作符的求值顺序被重新定义，以保证它们现在有确切的求值顺序：

* 对于

```cpp
    e1[e2]
    e1.e2
    e1.*e2
    e1->*e2
    e1<<e2
    e1>>e2
```

现在保证`e1`会在`e2`之前求值，因此求值顺序将是从左到右求值。

然而，注意同一个函数调用中不同参数的求值顺序仍然是未定义的，也就是说，在下面的表达式中：

```cpp
    e1.f(a1, a2, a3)
```

现在只保证`e1`的求值顺序在`a1`，`a2`，`a3`之前，但`a1`，`a2`和`a3`的求值顺序仍然是未定义的。

* 在所有带有赋值操作的运算符中：

```cpp
    e2=e1
    e2+=e1
    e2*=e1
    ...
```

现在保证右侧的`e1`会在左侧的`e2`之前求值。

* 最后，在像下面的`new`表达式中：

```cpp
    new Type(e)
```

现在保证会在对`e`求值之前完成内存分配，初始化操作保证会在使用到该变量的值之前完成。

所有这些保证都适用于基础类型和自定义类型，因此，自从C++17开始：

```cpp
std::string s = "I heard it even works if you don't believe";
s.replace(0,8,"").replace(s.find("even"),4,"always")
                 .replace(s.find("don't believe"), 9, "use C++17");
```

将保证`s`的值变为：

```cpp
    it always works if you use C++17
```

因此，每一个`find()`调用左边的所有替换操作都会在这个`find()`调用之前完成，另外，对于表达式：

```cpp
i = 0;
std::cout << ++i << ' ' << --i << '\n';
```

现在只要`i`的类型支持这些运算符，那么就会保证会输出1 0。

然而，大多数其他运算符的求值顺序依然是未定义的。例如：

```cpp
i = i++ + i;    //仍然是未定义行为
```

在这里，右侧的`i`的值可能是左侧的`i`的值，也可能是递增之后的值。

新的表达式求值顺序的另一个应用是函数内可以在传入的参数之前插入一个零值（见10.2.1节）。

## 7.3 更宽松的用整型初始化枚举值的规则

对于有一个固定类型的枚举类型，自从C++17开始你可以用一个整型值直接使用列表初始化枚举值。这可以用于带有明确类型的无作用域枚举和所有有作用域的枚举，因为它们都有以恶搞默认的相关类型：

```cpp
//有相关类型的无作用域枚举:
enum MyInt : char { };
MyInt i1{42};       //OK since C++17(ERROR before C++17)
MyInt i2 = 42;      //仍然错误
MyInt i3(42);       //仍然错误
MyInt i4 = {42};    //仍然错误

//带有默认相关类型的有作用域枚举:
enum class Salutation {mr, mrs};
Salutation s1{0};       //OK since C++17(ERROR before C++17)
Salutation s2 = 0;      //仍然错误
Salutation s3(0);       //仍然错误
Salutation s4 = {0};    //仍然错误
```

如果`Salutation`有明确相关类型也可以：

```cpp
//带有明确相关类型的有作用域的枚举
enum class Salutation : char {mr, mrs};
Salutation s1{0};       //OK since C++17(ERROR before C++17)
Salutation s2 = 0;      //仍然错误
Salutation s3(0);       //仍然错误
Salutation s4 = {0};    //仍然错误
```

对于没有明确相关类型的没有作用域的枚举（没有`class`的`enum`），你仍然不能用列表初始化：

```cpp
enum Flag {bit1=1, bit2=2, bit3=4};
Flag f1{0};         //仍然错误
```

注意列表初始化也仍然不允许窄化，所以你不能传递一个浮点值：

```cpp
enum MyInt : char { };
MyInt i5{42.2};     //仍然错误
```

这个特性诞生的目的是为了支持通过定义一个枚举类型来定义一种新的整数类型的技巧，就像上例中的`MyInt`一样。如果没有这个特性，在没有强制类型转换的情况下就不能初始化一个新的对象。

事实上，自从C++17开始，C++标准库也提供了`std::byte`(见第17章)，`std::byte`就直接利用了这个特性。

## 7.4 修复`auto`类型的列表初始化

自从在C++11中引入了花括号的统一初始化方式之后，每当使用`auto`代替明确类型时就会出现一些和直觉不一致的结果：

```cpp
int x{42};      //初始化一个int
int y{1,2,3};   //错误
auto a{42};     //初始化一个std::initializer_list<int>
auto b{1,2,3};  //OK:初始化一个std::initializer_list<int>
```

这些直接列表初始化（没有=的花括号初始化）的不一致行为已经被修复了，所以我们会有下面的行为：

```cpp
int x{42};      //初始化一个int
int y{1,2,3};   //错误
auto a{42};     //现在初始化一个int
auto b{1,2,3};  //现在是错误的
```

注意这是一个破坏性的更改，可能会导致程序的行为变得和以前不一致。出于这个原因，实现了这个更改的编译器通常即使在C++11的模式下仍然会应用该更改。对于主流编译器来说，这个更改被Visual Studio 2015，g++5和clang 3.8接受。

注意当使用`auto`作为类型时，拷贝列表初始化（带有=的花括号初始化）仍然总是会得到`std::initializer_list<>`类型：

```cpp
auto c = {42};      //仍然初始化为一个std::initializer_list<int>
auto d = {1,2,3};   //仍然OK:初始化一个std::initializer_list<int>
```

因此，现在直接列表初始化（没有=）和拷贝列表初始化（有=）的行为将可能有明显的不同：

```cpp
auto a{42};     //现在初始化一个int
auto c = {42};  //现在初始化一个std::initializer_list<int>
```

推荐的初始化变量的方式依然是使用直接列表初始化（没有=的花括号）。

## 7.5 十六进制浮点字面量

C++17标准化了明确的16进制浮点字面量（有些编译器在C++17之前就已经支持了）。当需要一个精确的浮点数表示时这个特性特别有用（因为十进制浮点数会转化为能表示的最接近的值，不能保证一定是精确的）。例如：

*lang/hexfloat.cpp*

```cpp
#include <iostream>
#include <iomanip>

int main()
{
    //初始化浮点数列表
    std::initializer_list<double> values{
        0x1p4,          //16
        0xA,            //10
        0xAp2,          //40
        5e0,            //5
        0x1.4p+2,       //5
        1e5             //100000
        0x1.86Ap+16     //100000
        0xC.68p+2       //49.625
    };

    //以十进制浮点数和十六进制浮点数格式输出值：
    for (double d : values) {
        std::cout << "dec: " << std::setw(6) << std::defaultfloat << d
        << " hex: " << std::hexfloat << d << '\n';
    }
}
```

这段程序以不同的已有方式和新的十六进制浮点数方式定义了一些浮点值。新的方式是以2为基数的科学计数法：

* 有效位以十六进制小数的格式书写
* 指数部分以10十进制格式书写，被解释为基数2的指数

例如，0xAp2是一种表示十进制40的方法(2的2次方再乘以10)。这个值也可以被表示为0x1.4p+5，也就是1.25*32（0x1.4的十进制是1.25, 2的5次方即为32）。

程序会有像下面的输出：

```cpp
    dec:        16  hex: 0x1p+4
    dec:        10  hex: 0x1.4p+3
    dec:        40  hex: 0x1.4p+5
    dec:         5  hex: 0x1.4p+2
    dec:         5  hex: 0x1.4p+2
    dec:    100000  hex: 0x1.86ap+16
    dec:    100000  hex: 0x1.86ap+16
    dec:    49.625  hex: 0x1.8dp+5
```

就像你在示例中看到的，对十六进制浮点数符号的支持已经在输出流中以`std::hexfloat`操作符的形式存在（自从C++11可用）。

## 7.6 UTF-8字符字面量

自从C++11开始，C++支持前缀u8代表UTF-8字符串字面量。但是，这个前缀却不适用于字符字面量。C++17修复了这个问题，因此你可以写：

```cpp
char c = u8'6';     //UTF-8编码的字符6
```

通过这种方式你可以保证字符c的值为UTF-8编码的字符6.你可以使用所有7字节的US-ASCII字符，这些字符编码和它们的UTF-8编码完全一样。也就是说，这样的写法对于7字节的US-ASCII, ISO Latin-1, ISO-8859-15, 和基础的Windows character set都能保证有正确的值。通常你的源码中的字符无论是US-ASCII还是UTF-8都是同样的字符因此这个前缀并不是必须的，例如字符c的值总会是54（十六进制表示为36）。

但有些背景下这个前缀是必要的：对于源代码中的字符和字符串字面量，C++只标准化了你可以用的字符但并没有规定它们的值。它们的值依赖于源代码的字符集。而且当编译器为它们生成代码时也是使用字符集。源代码的字符集总是7字节的US-ASCII编码因此通常可执行文件中的字符都是一样的；因此在任何C++程序中所有字符和字符串字面量（无论有没有u8前缀）都有相同的值。

但在非常少见的情况下可能并不是这样的。例如，在一台仍然在使用EBCDIC字符集旧的IBM主机中，字符'6'的值为246（十六进制表示为F6）。在一个使用EBCDIC字符集的程序中上面提到的字符类型的c的值将为246。若果在一台UTF-8编码平台的机器上运行这个程序可能会打印出字符ö,这个字符是ASCII中的值为246的字符。在这种情况下u8前缀就是必须的了。

注意u8前缀只能用于单个字符，且该字符在UTF-8中必须只有一个字节。一个像这样的初始化：

```cpp
char c = u8'ö';
```

是不允许的，因为德语中的ö在UTF-8中要占用两个字节，分别是195和182（十六进制表示为C3 B6）。

因此字符和字符串字面量现在都接受如下的前缀：

* 单字节的US-ASCII和UTF-8使用u8前缀
* 两个字节的UTF-16使用u前缀
* 四个字节的UTF-32使用U前缀
* 没有确定编码的宽字符用l前缀，它可能有两个或四个字节

## 7.7 异常明细作为类型的一部分

自从C++17开始异常明细成为了函数类型的一部分。也就是说，现在下面两个函数拥有不同的类型：

```cpp
void f1();
void f2() noexcept;     //不同的类型
```

在C++17之前，这两个函数有相同的类型。

因此，现在编译器会检查你是否在需要一个不抛出异常的函数时候给出了一个会抛出异常的函数：

```cpp
void (*fp)() noexcept;  //指向一个不抛出异常的函数
fp = f2;                //OK
fp = f1;                //自从C++17起错误
```

当然，在需要可以抛出异常的函数的时候给出不抛出异常的函数依然是有效的：

```cpp
void (*fp2)();      //指向一个可以抛出异常的函数
fp2 = f2;           //OK
fp2 = f1;           //OK
```

因此，新的特性并不会影响已有的没有使用`noexcept`声明的函数指针，但是请记住你现在不能再违反函数指针的`noexcept`要求。（这一点可能会影响现有的程序）

不允许根据异常明细的不同来重载签名相同的函数（也不允许只根据返回值的不同来重载函数）：

```cpp
void f3();
void f3() noexcept;     //错误
```

注意所有其他的规则都不受影响。例如，你仍然不能忽略基类虚函数声明中的`noexcept`声明：

```cpp
class Base {
  public:
    virtual void foo() noexcept;
    ...
};

class Derived : public Base {
  public:
    void foo() override;  //错误，没有重载
    ...
};
```

这里，派生类中的成员函数`foo()`有一个和基类中不同的类型，所以它不能重载基类中的`foo()`函数。这段代码仍然不能通过编译。尽管没有`override`声明也不能通过编译，因为我们不能用一个带有更宽松异常明细的函数来覆盖基类中的函数。

#### 使用有条件的异常明细

当使用有条件的`noexcept`声明时，函数的类型依赖于条件的真假：

```cpp
void f1();
void f2() noexcept;
void f3() noexcept(sizeof(int)<4);  //和f1()或f2()的类型相同
void f4() noexcept(sizeof(int)>4);  //和f3()的类型不同
```

在这里，`f3()`的类型取决于代码编译时条件的真假：

* 如果`sizeof(int)`>=4，那么最后的结果是

```cpp
    void f3() noexcept(false);  //和f1()类型相同
```

* 如果`sizeof(int)`<4，那么最后的结果是

```cpp
    void f3() noexcept(true);   //和f3()类型相同
```

因为`f4()`的异常明细条件和`f3()`相反，所以`f4()`的类型总是和`f3()`不同。

“旧风格”的空`throw`声明仍然可以使用但自从C++17开始就被废弃了：

```cpp
void f5() throw();  //和void f5() noexcept相同但是这种写法已经被废弃
```

动态的`throw`声明已经不再被支持了（自从C++17开始被废弃）：

```cpp
void f6() throw(std::bad_alloc);    //错误：自从C++17开始无效
```

#### 对泛型库的影响

将`noexcept`声明作为类型的一部分可能会对现有的泛型库造成一些影响。例如，下面的代码到C++14还是有效的，但对C++17来说不能通过编译：

*lang/noexceptcalls.cpp*

```cpp
#include <iostream>

template<typename T>
void call(T op1, T op2)
{
    op1();
    op2();
}

void f1() {
    std::cout << "f1()\n";
}
void f2() noexcept {
    std::cout << "f2()\n";
}

int main()
{
    call(f1, f2);   //自从C++17起错误
}
```

问题在于自从C++17开始`f1()`和`f2()`有了不同的类型所以当实例化模板`call()`时编译器无法再为两者找到共同的类型`T`。

在C++17中你可以用两个不同的类型来处理这种情况：

```cpp
template<typename T1, typename T2>
void call(T1 op1, T2 op2)
{
    op1();
    op2();
}
```

如果你希望或者不得不覆盖所有可能的函数类型，你将必须定义双倍的版本。一个例子是标准的类型特征模板`std::is_function<>`的定义，基础的模板定义假设`T`不是一个函数：

```cpp
//基础的模板定义(假设T不是一个函数)
template<typename T> struct is_function : std::false_type { };
```

这个模板继承自`std::false_type`（见20.3节）因此`is_function<T>::value`对于任何类型`T`会产出false。

对于任何函数类型，有一个部分特化的类型存在，它继承自`std::true_type`（见20.3节）因此其成员`value`总是返回true：

```cpp
//对所有函数类型的部分特化版本
template<typename Ret, typename... Params>
struct is_function<Ret (Params...)> : std::true_type { };

template<typename Ret, typename... Params>
struct is_function<Ret (Params...) const> : std::true_type { };

template<typename Ret, typename... Params>
struct is_function<Ret (Params...) &> : std::true_type { };

template<typename Ret, typename... Params>
struct is_function<Ret (Params...) const &> : std::true_type { };
...
```

在C++17之前，已经有了24个部分特化版本，因为函数类型可以带有`const`和`volatile`修饰词，还有左值(&)和右值(&&)引用修饰词，而且你还需要处理带有变长参数的版本。

现在，有了C++17，部分特化的版本数又要翻倍，因为又添加了一个`noexcept`修饰符，所以现在有48个部分特化版本：

```cpp
...
//为所有带有noexcept修饰符的函数定义的部分特化版本：
template<typename Ret, typename... Params>
struct is_function<Ret (Params...) noexcept> : std::true_type { };

template<typename Ret, typename... Params>
struct is_function<Ret (Params...) const noexcept> : std::true_type { };

template<typename Ret, typename... Params>
struct is_function<Ret (Params...) & noexcept> : std::true_type { };

template<typename Ret, typename... Params>
struct is_function<Ret (Params...) const& noexcept> : std::true_type { };
```

没有`noexcept`重载版本的编译器实现可能不会编译通过那些使用了`noexcept`传递的函数。

## 7.8 单参数`static_assert`

自从C++17开始，以前`static_assert()`需要的诊断信息数变为了可选的。这意味着最后的诊断信息将完全依赖于平台，例如：

```cpp
#include <type_traits>

template<typename T>
class C {
    //OK since C++11
    static_assert(std::is_default_constructible<T>::value,
                  "class C: elements must be default-constructible");

    //OK since C++17
    static_assert(std::is_default_constructible_v<T>);
    ...
};
```

新的没有诊断信息的断言版本还用了带有`_v`后缀的新的类型特征（见20.1节）。

## 7.9 预处理条件`__has_include`

C++17扩展了预处理指令的检查能力，现在可以检查一个特定的头文件是否被已经包含了。例如：

```cpp
#if __has_include(<filesystem>)
#  include <filesystem>
#  define HAS_FILESYSTEM 1
#elif __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
#  define HAS_FILESYSTEM 1
#  define FILESYSTEM_IS_EXPERIMENTAL 1
#elif __has_include("filesystem.hpp")
#  include "filesystem.hpp"
#  define HAS_FILESYSTEM 1
#  define FILESYSTEM_IS_EXPERIMENTAL 1
#else
#  define HAS_FILESYSTEM 0
#endif
```

如果#include HEADER指令是有效的（或者说不会产生错误），那么`__has_include(HEADER)`就会返回1。即该指令并不是真的检查是否包含过指定头文件，而是判断包含该头文件会不会产生错误，如果不会那么就认为没有包含过该头文件。

## 7.10 后记

嵌套命名空间定义（见7.1节）最早由Jon Jagger在[https://wg21.link/n1524](https://wg21.link/n1524)上提出，2014年Robert Kawulak在[https://wg21.link/n4026](https://wg21.link/n4026)上给出了一个新的提议。最终被接受的正式提案由Robert Kawulak和Andrew Tomazos在[https://wg21.link/n4230](https://wg21.link/n4230)发表。

精确的表达式求值顺序（见7.2节）最早由Gabriel Dos Reis, Herb Sutter和Jonathan Caves在[https://wg21.link/n4228](https://wg21.link/n4228)上提出。最终被接受的正式提案由Gabriel Dos Reis, Herb Sutter和Jonathan Caves在[https://wg21.link/p0145r3](https://wg21.link/p0145r3)发表。

更宽松的枚举类型初始化（见7.3节）最早由Gabriel Dos Reis在[https://wg21.link/p0138r0](https://wg21.link/p0138r0)上提出。最终被接受的正式提案由Gabriel Dos Reis在[https://wg21.link/p0138r2](https://wg21.link/p0138r2)发表。

修复用`auto`推断列表初始化（见7.4节）最早由Ville Voutilainen在[https://wg21.link/n3681](https://wg21.link/n3681)和[https://wg21.link/3912](https://wg21.link/3912)上提出。最终的提案由James Dennett在[https://wg21.link/n3681](https://wg21.link/n3681)上提出。

十六进制浮点字面量（见7.5节）最早由Thomas Köppe在[https://wg21.link/p0245r0](https://wg21.link/p0245r0)上提出。最终被接受的正式提案由Thomas Köppe在[https://wg21.link/p0245r1](https://wg21.link/p0245r1)发表。

UTF-8字符字面量前缀（见7.6节）最早由Richard Smith在[https://wg21.link/n4197](https://wg21.link/n4197)上提出。最终被接受的正式提案由Richard Smith在[https://wg21.link/n4267](https://wg21.link/n4267)发表。

异常明细作为函数类型的一部分（见7.7节）最早由Jens Maurer在[https://wg21.link/n4320](https://wg21.link/n4320)上提出。最终被接受的正式提案由Jens Maurer在[https://wg21.link/p0012r1](https://wg21.link/p0012r1)发表。

单参数`static_assert`（见7.8节）最终被接受的正式提案由Walter E.Brown在[https://wg21.link/n3928](https://wg21.link/n3928)发表。

预处理语句`__has_include()`（见7.9节）最早由Clark Nelson和Richard Smith作为[https://wg21.link/p0061r0](https://wg21.link/p0061r0)的一部分提出。最终被接受的正式提案由 Clark Nelson和Richard Smith在[https://wg21.link/p0061r1](https://wg21.link/p0061r1)发表。

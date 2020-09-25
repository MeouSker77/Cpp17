# Chapter 03 内联变量

C++的一个优势在于可以在只有头文件的情况下完成库的部署。然而，在C++17之前，只有当这个库既不需要全局变量也不提供全局变量的时候才可以这么做。

自从C++17开始你可以在头文件中以内联的方式定义一个变量或者对象，如果该变量被多处引用，那么所有引用将会指向同一个对象：

```cpp
class MyClass {
    static inline std::string name = "";    //OK since C++17
    ...
};

inline MyClass myGlobalObj; //即便被多个cpp文件包含或定义也OK
```

## 3.1 内联变量产生的动机

在以往的C++里，不允许在类里直接初始化非常量静态成员：

```cpp
class MyClass {
    static std::string name = "";   //编译期错误
    ...
};
```

如果在类定义的外部定义该成员的话，那么当该头文件被多个`cpp`文件包含的时候仍然会引发错误：

```cpp
class MyClass {
    static std::string name;    //OK
    ...
};
std::string MyClass::name = "";     //如果该头文件被多个cpp文件包含，链接时会报错
```

根据一次定义原则(ODR)，一个变量或实体只能在一个单元里被定义一次。即便是使用预处理器也没有用：

```cpp
#ifndef MYHEADER_HPP
#define MYHEADER_HPP

class MyClass {
    static std::string name;    //OK
    ...
};
std::string MyClass::name = "";     //如果被多个cpp文件包含，链接时会报错

#endif
```

问题不在于头文件可能被重复包含，而在于不同的`cpp`文件都包含了这个头文件，然后都定义了一个`MyClass::name`，这违反了一次定义原则。

出于同样的原因，如果你在头文件里定义了一个类，而且你还在多个`cpp`文件里包含这个头文件，那么当链接时你会遇到一个错误：

```cpp
class MyClass {
    ...
};
MyClass myGlobalObject;     //如果被多个cpp文件包含，链接时会报错
```

#### 解决方法

在一些情况下，是有解决方法的：

* 你可以在一个`class`/`struct`里初始化一个常量静态成员：

```cpp
class MyClass {
    static const bool trace = false;
    ...
};
```

* 你可以定义一个返回静态局部变量的内联函数：

```cpp
inline std::string getName() {
    static std::string name = "initial value";
    return name;
}
```

* 你可以定义一个返回静态变量的成员函数：

```cpp
std::string getMyGlobalObject() {
    static std::string myGlobalObject = "initial value";
    return myGlobalObject;
}
```

* 你可以使用变量模板（C++14引入）：

```cpp
template<typename T = std::string>
T myGlobalObject = "initial value";
```

* 你可以从一个模板基类中继承静态成员：

```cpp
template<typename Dummy>
class MyClassStatics
{
    static std::string name;
};

template<typename Dummy>
std::string MyClassStatics<Dummy>::name = "initial value";

class MyClass : public MyClassStatics<void>
{
    ...
};
```

但是所有这些方法都会导致签名重载，还会导致可读性变差，有些还会导致变量的使用方法发生变化。另外，全局变量的初始化可能会推迟到它第一次被使用的时候，所以那些假设变量一开始就已经初始化的写法是不可行的（例如使用一个对象来监控整个程序的过程）。

## 3.2 使用内联变量

现在，有了内联变量，当你在头文件中定义对象时可以直接对它的静态成员进行初始化，即便该头文件可能被多个cpp文件包含：

```cpp
class MyClass {
    static inline std::string name = "";    //OK since C++17
};

inline MyClass myGlobalObj;     //即使被多个cpp文件包含也OK
```

当第一个包含内联变量的翻译单元进行编译时会初始化内联变量。

这里使用的`inline`和函数声明时用到的`inline`有相同的语义：

* 它可以在多个翻译单元中定义，只要所有定义保持一致
* 它必须在所有用到它的翻译单元中定义

当两个cpp文件从同一个头文件中获得定义时两个条件都可以得到满足。程序的行为就好像只有一个变量一样，所有的引用都指向这同一个变量。

你甚至可以利用这个特性在头文件中定义原子类型：

```cpp
inline std::atomic<bool> ready{false};
```

注意像往常一样你必须在定义`std::atomic`变量的时候初始化它们。

注意你仍然必须确保初始化变量之前其类型是完整的。例如，如果一个`struct`或`class`中有一个和它自己同类型的静态成员，那么这个成员只能在类型声明之后进行`inline`定义：

```cpp
struct MyValue {
    int value;
    MyValue(int i) : value{i} {
    }
    //一个存储该类型最大值的静态变量
    static const MyValue max;   //在这里只能声明，不能定义
};
inline const MyValue MyValue::max = 1000;
```

## 3.3 `constexpr`现在隐含着`inline`

对于静态数据成员，`constexpr`现在隐含着`inline`，例如如下声明自从C++17之后会定义数据成员`n`：

```cpp
struct D {
    static constexpr int n = 5;     //C++11/C++14: 这是一个声明语句
                                    //C++17: 这是一个定义语句
};
```

也就是说，这和下面的写法一样：

```cpp
struct D {
    inline static constexpr int n = 5;
};
```

注意在C++17之前你就可以只声明而没有相应的定义：

```cpp
struct D {
    static constexpr int n = 5;
};
```

但是只有当不需要`D::n`的定义时这种写法才有效，例如，如果要取`D::n`的值这种写法是没问题的：

```cpp
std::cout << D::n;      //OK(ostream::operator<<(int)获取了D::n的值)
```

但如果需要取`D::n`的引用时上述写法就会导致问题：

```cpp
int inc(const int& i);

std::cout << inc(D::n);     //通常会导致错误
```

这段代码违反了一次定义原则（ODR），因为该代码中`D::n`没有定义。当开启编译器的优化选项构建它的时候，也许它能正常工作或者也可能在链接时给出一个缺少定义的错误。当不开启优化进行构建的时候，它一般会给出一个缺少`D::n`定义的错误。

因此，在C++17之前，你必须在一个翻译单元内明确定义`D::n`

```cpp
constexpr int D::n;     //C++11/C++14: 定义
                        //since C++17: 多余的定义(废弃)
```

当用C++17的标准去构建这段代码的时候，类里面的声明自动的变成了一个定义，因此这段代码现在即便没有显式的定义也是正确的，显式定义虽然还有效但已经是被废弃的写法。

## 3.4 `inline`变量和`thread_local`

通过使用`thread_local`你也可以使每个线程拥有自己的`inline`变量：

```cpp
struct ThreadData {
    inline static thread_local std::string name;  //每个线程都有自己的name
    ...
};

inline thread_local std::vector<std::string> cache; //每个线程都有一份cache
```

作为一份完整的示例，考虑下面的头文件：

*lang/inlinethreadlocal.hpp*

```cpp
#include<string>
#include<iostream>

struct MyData {
    inline static std::string gName = "global";     //一个程序只有一个该变量
    inline static thread_local std::string tName = "tls";  //每个线程有一个该变量
    std::string lName = "local";                    //每个该类型的对象有一个该变量
    ...
    void print(const std::string& msg) const {
        std::cout << msg << '\n';
        std::cout << "- gName: " << gName << '\n';
        std::cout << "- tName: " << tName << '\n';
        std::cout << "- lName: " << lName << '\n';

    }
};

inline thread_local MyDate myThreadData;    //每个线程有一个该对象
```

你可以将该头文件应用于如下的包含`main`函数的翻译单元里：

*lang/inlinethreadlocal1.cpp*

```cpp
#include<inlinethreadlocal.hpp>
#include<thread>

void foo();

int main()
{
    myThreadData.print("main() begin:");
    myThreadData.gName = "thread1 name";
    myThreadData.tName = "thread1 name";
    myThreadData.lName = "thread1 name";
    myThreadData.print("main() later:");

    std::thread t(foo);
    t.join();
    myThreadData.print("main() end:");
}
```

你也可以在另一个定义`foo()`的翻译单元里使用这个头文件，`foo()`将在一个不同的线程里被调用：

*lang/inlinethreadlocal2.cpp*

```cpp
#include "inlinethreadlocal.hpp"

void foo()
{
    myThreadData.print("foo() begin:");

    myThreadData.gName = "thread2 name";
    myThreadData.tName = "thread2 name";
    myThreadData.lName = "thread2 name";
    myThreadData.print("foo() end:");
}
```

程序会有如下输出：

```cpp
    main() begin:
    - gName: global
    - tName: tls
    - lName: local
    main() later:
    - gName: thread1 name
    - tName: thread1 name
    - lName: thread1 name
    foo() begin:
    - gName: thread1 name
    - tName: tls
    - lName: local
    foo() end:
    - gName: thread2 name
    - tName: thread2 name
    - lName: thread2 name
    main() end:
    - gName: thread2 name
    - tName: thread1 name
    - lName: thread1 name
```

## 3.5 使用`inline`变量追踪`::new`

下面的程序演示了怎么使用`inline`变量跟踪分配器的调用：

*lang/tracknew.hpp*

```cpp
#ifndef TRACKNEW_HPP
#define TRACKNEW_HPP

#include <new>
#include <cstdlib>  //for malloc()
#include <iostream>

class TrackNew {
private:
    static inline int numMalloc = 0;    //malloc调用的次数
    static inline long sumSize = 0;     //malloc分配的总内存
    static inline bool doTrace = false; //允许跟踪
    static inline bool inNew = false;   //在重载的分配器里不要输出跟踪信息
public:
    //重置 new 计数
    static void reset() {
        numMalloc = 0;
        numSize = 0;
    }

    //允许打印每一次new的输出
    static void trace(bool b) {
        doTrace = b;
    }

    //打印当前的状态
    static void status() {
        std::cerr << numMalloc << " mallocs for "
                  << sumSize << " Bytes" << '\n';
    }

    //有跟踪功能的分配器的实现
    static void* allocate(std::size_t size, const char* call) {
        //输出追踪信息的时候有可能再次分配内存，处理这种情况
        if (inNew) {
            return std::malloc(size);
        }
        inNew = true;
        //跟踪分配器的调用并输出信息
        ++numMalloc;
        sumSize += size;
        void * p = std::malloc(size);
        if (doTrace) {
            std::cerr << "#" << numMalloc << " "
                      << call << " (" << size << " Bytes) => "
                      << p << "  (total: " << sumSize << " Bytes)" << '\n';
        }
        inNew = false;
        return p;
    }
};

inline void* operator new (std::size_t size)
{
    return TrackNew::allocate(size, "::new");
}

inline void* operator new[] (std::size_t size)
{
    return TrackNew::allocate(size, "::new[]");
}

#endif  //TRACKNEW_HPP
```

考虑在下面的头文件里使用这个头文件：

*lang/tracknewtest.hpp*

```cpp
#include "tracknew.hpp"
#include <string>

class MyClass {
    static inline std::string name = "initial name with 26 chars";
    ...
};

MyClass myGlobalObj;    //自从C++17之后即使被多个cpp文件包含也OK
```

然后是`cpp`文件：

*lang/tracknewtest.cpp*

```cpp
#include "tracknew.hpp"
#include "tracknewtest.hpp"
#include <iostream>
#include <string>

int main()
{
    TrackNew::status();
    TrackNew::trace(true);
    std::string s = "an string value with 29 chars";
    TrackNew::status();
}
```

输出结果取决于追踪器被初始化的时间和初始化时有多少分配操作。但输出的最后一部分应该像下面这样：

```cpp
    ...
    #33 ::new (27 Bytes) => 0x89dda0  (total: 2977 Bytes)
    33 mallocs for 2977 Bytes
    #34 ::new (30 Bytes) => 0x89db00  (total: 3007 Bytes)
    34 mallocs for 3007 Bytes
```

其中27字节用于`MyClass::name`的初始化，30字节用于`main`函数内的`s`。（注意这里特意使字符串`s`的长度大于15，以避免有些编译器使用短字符串优化(SSO)导致`s`不在堆上分配内存。短字符串优化指某些编译器在`string`类里预先设置一个长度最大到15的成员，然后用该成员来存储长度小于15的字符串，而不是在堆上分配内存）。

## 3.6 后记

内联变量的想法最早由David Krauss在[https://wg21.link/n4147](https://wg21.link/n4147)上提到，并由Hal Finkel和Richard Smith在[https://wg21.link/n4424](https://wg21.link/n4424)上第一次正式提出。最终被接受的正式提案由Hal Finkel和Richard Smith在[https://wg21.link/p0386r2](https://wg21.link/p0386r2)发表。

# Chapter 04 聚合体扩展

列表初始化是C++里初始化对象的一种方式，它允许用花括号包围的多个值来初始化一个聚合体。

```cpp
struct Data {
    std::string name;
    double value;
};
```

自从C++17开始，列表初始化也可以用于派生的聚合体：

```cpp
struct MoreDate : Data {
    bool done;
};

MoreData y{{"test", 6.778}, false};
```

如你所见，列表初始化现在支持用嵌套的花括号来给聚合体的基类传递值。

## 4.1 聚合体扩展产生的动机

在这个特性出现之前，派生类是不允许使用列表初始化的，因此你必须定义一个构造函数：

```cpp
struct Cpp14Data : Data {
    bool done;
    Cpp14Data (const std::string& s, double d, bool b)
        : Data{s,d}, done{b} {
    }
};

Cpp14Data y{"test1", 6.778, false};
```

有了这个特性后我们就可以直接使用列表初始化，这样做的一个好处是你可以清楚的看到哪些值被传递给基类：

```cpp
MoreData y{{"test1", 6.778}, false};
```

## 4.2 使用聚合体扩展

该特性的一个典型应用场景是用来初始化一个从C风格结构体继承而来的类或结构体。例如：

```cpp
struct Data {
    const char* name;
    double value;
};

struct PData : Data {
    bool critical;
    void print() const {
        std::cout << '[' << name << ',' << value << "]\n";
    }
};

PData y{{"test1", 6.778}, false};
y.print();
```

在这里，嵌套的花括号内部的参数被传递给基类`Data`。

注意，你也可以跳过初始化某些值。在这种情况下那些元素都会被初始化为0值（类类型会调用默认构造函数，基础类型会初始化为0值）。例如：

```cpp
PData a{};              //所有元素都被初始化为0值
PData b{{"msg"}}        //和{{"msg", 0.0}, false}一样
PData c{{}, true};      //和{{nullptr, 0.0}, true}一样
PData d;                //基础类型的值不确定
```

注意使用空花括号和不使用花括号完全不同：

* 使用空花括号会将所有成员初始化为0值，其中的`string`类型的`name`会调用默认构造函数，`double`类型的`value`会初始化为0.0，`bool`类型的`flag`会初始化为false。
* 如果像`d`那样不使用花括号，那么`string`类型的`name`会调用默认构造函数，但所有其他成员都不会进行初始化，因此它们的值都不确定。

你也可以从一个非聚合体派生出一个新的类，然后对该类使用列表初始化。例如：

```cpp
struct MyString : std::string {
    void print() const {
        if (empty()) {
            std::cout << "<undefined>\n";
        }
        else {
            std::cout << c_str() << '\n';
        }
    }
};

MyString y{{"test1"}};
```

你甚至可以从多个聚合体或非聚合体派生新的类然后使用列表初始化：

```cpp
template<typename T>
struct D : std::string, std::complex<T>
{
    std::string data;
};
```

对于这个类你可以用如下的方式初始化它：

```cpp
D<float> s{{"hello"}, {4.5, 6.7}, "world"};         //OK since C++17
std::cout << s.data;                                //输出"world"
std::cout << static_cast<std::string>(s);           //输出"hello"
std::cout << static_cast<std::complex<float>>(s);   //输出(4.5, 6.7)
```

其中嵌套的初始化列表按照基类声明的顺序被传递给各个基类。

这个新的特性也可以帮助我们用非常少的代码来重载lambdas（见13.1节）

## 4.3 聚合体的定义

总的来说，自从C++17开始聚合体的定义为:

* 要么是一个数组
* 要么是一个带有如下特征的类类型（`class`，`struct`或`union`）:

    \- 没有显式声明的或者`explicit`的构造函数

    \- 没有用`using`声明引入继承的构造函数

    \- 没有`private`或`protected`的非静态数据成员

    \- 没有`virtual`函数

    \- 没有`virtual`，`private`或`protected`的基类

为了能使用列表初始化还必须保证初始化过程中没有`private`或`protected`的基类成员和构造函数。

C++17引入了一个新的类型特征`is_aggregate<>`（见20.2节）用来测试一个类型是否是聚合体：

```cpp
template<typename T>
struct D : std::string, std::complex<T> {
    std::string data;
};
D<float> s{{"hello"}, {4.5, 6.7}, "world"};         //OK since C++17
std::cout << std::is_aggregate<decltype(s)>::value; //输出1（true）
```

## 4.4 向后的兼容性

注意下面的代码不再能通过编译：

*lang/aggr14.cpp*

```cpp
struct Derived;

struct Base {
    friend struct Derived;
private:
    Base() {}
};

struct Derived : Base {
};

int main()
{
    Derived d1{};       //ERROR since C++17
    Derived d2;         //仍然OK（但可能不会初始化）
}
```

在C++17之前，`Derived`不是一个聚合类，因此，

```cpp
Derived d1{};
```

会调用`Derived`的隐式定义的构造函数，该函数会默认调用基类`Base`的默认构造函数。尽管基类的默认构造函数是`private`的，但`Derived`是`Base`的友元，因此依然可以访问`Base`的默认构造函数，所以该语句是正确的。

自从C++17，这个例子中的`Derived`是一个聚合体，而且没有隐式的默认构造函数（构造函数没有通过`using`声明继承）。因此这个初始化是一个聚合体初始化，然而聚合体初始化不允许调用基类的私有构造函数，因此该语句就变成错误的了。这和`Derived`是否是友元没有关系。

## 4.5 后记

扩展的聚合体初始化特性最早由Oleg Smolsky在[https://wg21.link/n4404](https://wg21.link/n4404)上提出，最终被接受的正式提案由Oleg Smolsky在[https://wg21.link/p0017r1](https://wg21.link/p0017r1)发表。

类型特征`std::is_aggregate<>`作为美国国家机构对C++17标准的一个注释引入（见[https://wg21.link/lwg2911](https://wg21.link/lwg2911)）。

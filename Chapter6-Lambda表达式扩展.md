# Lambda 表达式扩展

lambda表达式是一个很大的成功，它最早在C++11中引入，在C++14中又引入了泛型lambda。它允许我们将函数作为参数传递，这让我们能更轻易的指明一种行为。

C++17强化了lambda表达式，使得它们可以在更多的场景应用：

* 在常量表达式中使用（例如，在编译期间）
* 在你需要当前对象的一份拷贝的时候使用（当在多线程中调用lambda时）

## 6.1 constexpr Lambda

自从C++17，只要有可能lambda就会隐式的声明`constexpr`。也就是说，任何只使用有效的编译期上下文（例如，只有字面量，没有静态变量，没有虚函数，没有`try`/`catch`，没有`new`/`delete`的上下文）的lambda都可以被用于编译期。

例如，你可以使用lambda表达式来计算一个在编译期间以值传递的参数的平方，并将结果用于指明`std::array<>`的元素数量。

```cpp
auto squared = [](auto val) {   //自从C++17起隐式声明为constexpr
    return val*val;
};

std::array<int, squared(5)>     //OK since C++17 => std::array<int, 25>
```

使用`constexpr`不允许的上下文内容将会使lambda失去作为`constexpr`表达式的能力，但你仍然可以在运行时上下文中使用lambda：

```cpp
auto squared2 = [](auto val) {  //自从C++17起隐式声明为constexpr
    static int calls = 0;       //OK，但是会使这个lambda失去作为constexpr的能力
    ...
    return val*val;
};
std::array<int, squared2(5)> a;     //错误，编译期上下文中有静态变量
std::cout << squared2(5) << '\n';   //OK
```

如果想要知道一个lambda在编译期上下文中是否是有效的，你可以声明它为一个`constexpr`：

```cpp
auto squared3 = [](auto val) constexpr {    //OK since C++17
    return val*val;
};
```

如果要指明确切的返回类型的话就像下面这样写：

```cpp
auto squared3i = [](int val) constexpr -> int { //OK since C++17
    return val*val;
};
```

关于`constexpr`函数的规则也适用于lambda：如果一个lambda表达式被用于运行时上下文，那么相应的功能也会在运行时被执行。然而，在一个无效的编译期上下文环境中使用`constexpr` lambda特性将会导致一个编译期错误：

```cpp
auto squared4 = [](auto val) constexpr {
    static int calls=0;     //错误，编译期上下文中不能有静态变量
    ...
    return val*val;
};
```

对于一个隐式或显式的`constexpr` lambda，函数调用符也是`constexpr`。也就是说，如下定义：

```cpp
auto squared = [](auto val) {       //自从C++17起隐式声明为constexpr
    return val*val;
};
```

将会转换成如下闭包类型：

```cpp
class ComplilerSpecificName {
  public:
    ...
    template<typename T>
    constexpr auto operator() (T val) const {
        return val*val;
    }
};
```

注意此处生成的闭包类型的函数调用符自动就是`constexpr`。自从C++17起，如果lambda显式或隐式的声明为`constexpr`，那么自动生成的函数调用运算符就也为`constexpr`。

## 6.2 向lambda传递this的拷贝

当在成员函数中使用lambda时，你没有隐式的方法访问调用该成员函数的实例。也就是说，在lambda中，如果没有捕获`this`，那么你将不能使用这个对象的成员（和你是否用`this->`引用成员没有关系）：

```cpp
class C {
  private:
    std::string name;
  public:
    ...
    void foo() {
        auto l1 = [] {std::cout << name << '\n'; }; //错误
        auto l2 = [] {std::cout << this->name << '\n'; }; //错误
        ...
    }
};
```

在C++11和C++14中，你必须以值或引用的方式捕获`this`：

```cpp
class C {
  private:
    std::string name;
  public:
    ...
    void foo() {
      auto l1 = [this] { std::cout << name << '\n'; }; //OK
      auto l2 = [=] { std::cout << name << '\n'; };    //OK
      auto l3 = [&] { std::cout << name << '\n'; };    //OK
      ...
    }
};
```

然而这里的问题是即便你是用拷贝方式捕获`this`，你捕获的也是该对象的一个引用（因为只有一个`this`指针会被拷贝）。当lambda的生存周期超过调用该成员函数的实例的生存周期时就会导致问题，比如一个极端的例子是在lambda中调用一个新的线程。调用新线程时正确的做法是传递整个对象的拷贝来避免并发和生存周期的问题，而不是传递该对象的引用。而且有时你也可能只是希望能获得当前实例的拷贝。

从C++14起中有一种变通的方法来做到这一点，但它的可读性和效果都比较差：

```cpp
class C {
  private:
    std::string name;
  public:
    ...
    void foo() {
        auto l1 = [thisCopy=*this] { std::cout << thisCopy.name << '\n'; };
        ...
    }
};
```

例如，程序员有时会在也捕获了其他对象时无意中使用`this`：

```cpp
auto l1 = [&, thisCopy=*this] {
    thisCopy.name = "new name";
    std::cout << name << '\n';  //OOPS：仍然是旧的name
};
```

自从C++17，你可以显式的使用`*this`来捕获当前对象的一个拷贝：

```cpp
class C {
  private:
    std::string name;
  public:
    ...
    void foo() {
        auto l1 = [*this] { std::cout << name << '\n'; };
        ...
    }
};
```

此处捕获列表中的`*this`意味着一个当前对象的拷贝被传递到了lambda中。你仍然可以和`*this`一起捕获其他对象，只要它们和`*this`没有冲突:

```cpp
auto l2 = [&, *this] {...};     //OK
auto l3 = [this, *this] {...};  //错误
```

这里有一份完整的示例：

*lang/lambdathis.cpp*

```cpp
#include <iostream>
#include <string>
#include <thread>

class Data {
  private:
    std::string name;
  public:
    Data(const std::string& s) name(s) {}
    atuo startThreadWithCopyOfThis() const {
        //三秒之后启动并返回一个新的线程
        using namespace std::literals;
        std::thread t([*this] {
            std::this_thread::sleep_for(3s);
            std::cout << name << '\n';
        });
        return t;
    }
};

int main()
{
    std::thread t;
    {
        Data d{"c1"};
        t = d.startThreadWithCopyOfThis();
    }   //d不再有效
    t.join();
}
```

例子中的lambda捕获列表中是`*this`，意味着将捕获一份拷贝。因此，即便对象`d`马上会被析构也不会有任何问题。

如果我们使用了[`this`]，[=]或[&]进行捕获，那么新启动的线程就会陷入不确定的行为，因为当lambda中要打印`name`的时候`name`所属的实例已经被销毁了。

## 6.3 后记

`constexpr` lambda最早由Faisal Vali, Ville Voutilainen和Gabriel Dos Reis在[https://wg21.link/n4487](https://wg21.link/n4487)上提出。最终被接受的正式提案由Faisal Vali, Jens Maurer和Richard Smith在[https://wg21.link/p0170r1](https://wg21.link/p0170r1)发表。

在lambda中捕获`*this`最早由H.Carter Edwards, Christian Trott, Hal Finkel Jim Reus, Robin Maffeo和Ben Sander在[https://wg21.link/p0018r0](https://wg21.link/p0018r0)上提出，最终被接受的正式提案由H.Carter Edwards, Daveed Vandevoorde, Christian Trott, Hal Finkel, Jim Reus, Robin Maffeo和Ben Sander在[https://wg21.link/p0180r3](https://wg21.link/p0180r3)发表。

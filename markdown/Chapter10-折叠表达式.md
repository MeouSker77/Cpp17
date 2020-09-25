# Chapter 10 折叠表达式

自从C++17开始，有了一个特性来方便的计算用一个二元操作符遍历一个参数包的所有参数的结果（带有一个可选的初始值）。

例如，下面的函数会返回所有参数之和：

```cpp
template<typename... T>
auto foldSum (T... args) {
    return (... + args);    //((arg1 + args2) + arg3)...
}
```

注意返回值语句两端的括号是折叠表达式的一部分，不可以被省略。

像下面这样调用函数：

```cpp
foldSum(47, 11, val, -1);
```

实例化出的模板将会变为：

```cpp
return 47 + 11 + val + -1;
```

还要注意折叠表达式的参数顺序可以不同，结果也可能不同（可能看起来有点反直觉）：像下面这样写，

```cpp
(... + args)
```

会导致

```cpp
((arg1 + arg2) + args3) ...

这意味着它重复向尾部添加新参数。你也可以写：

```cpp
(args + ...)
```

这将会重复向首部添加新参数，因此结果表达式是：

```cpp
(arg1 + (arg2 + arg3)) ...
```

## 10.1 折叠表达式产生的原因

折叠表达式避免了模板对所有参数进行递归展开。在C++17之前，你必须这么实现：

```cpp
template<typename T>
auto foldSumRec (T arg) {
    return arg;
}

template<typename T1, typename... Ts>
auto foldSumRec (T1 arg1, Ts... otheraArgs) {
    return arg1 + foldSumRec(otherArgs...);
}
```

这样的实现不仅写起来很笨重，还会给C++编译器带来很大压力。通过写成

```cpp
template<typename... T>
auto foldSum (T... args) {
    return (... + args);    //arg1 + arg2 + arg3 ...
}
```

无论对程序员还是编译器来说效果都更好。

## 10.2 使用折叠表达式

给定一个参数`args`和一个操作符`op`，C++17允许我们写：

* 一元左折叠

```cpp
(... op args)
```

这会被拓展为：*((arg1 **op** arg2) **op** arg3) **op*** ...

* 或者一元右折叠

```cpp
(args op ...)
```

这会被拓展为：*arg1 **op** (arg2 **op** ...(argN-1 **op** argN))*

括号是必须的，然而括号和...之间的空格不一定要有空格分割。

左折叠和右折叠的不同往往比想象中的更大。例如，即便是使用+也可能产生不同的效果。当使用左折叠时：

```cpp
template<typename T>
auto foldSumL(T... args) {
    return (... + args) //((arg1 + arg2) + arg3)...
}
```

如下调用：

```cpp
foldSumL(1, 2, 3)

```

等价于

```cpp
((1 + 2) + 3)
```

这也就意味下面的例子能够编译：

```cpp
std::cout << foldSumL(std::string("hello"), "world", "!") << '\n';  //OK
```

但是要记住只有当两边有一边是`std::string`时才能使用+。因为此处使用了左折叠，因此调用等价于

```cpp
(std::string("hello") + "world") + "!"
```

第一个括号会返回一个`std::string`，所以能够继续和"!"进行+运算。

然而，像这样的调用：

```cpp
std::cout << foldSumL("hello", "world", std::string("!")) << '\n';  //错误
```

将不能通过编译，因为它等价于

```cpp
("hello" + "world") + std::string("!")
```

对两个字符串字面量使用+是错误的。

然而，如果我们将实现改为：

```cpp
template<typename... T>
auto foldSumR(T... args) {
    return (args + ...);    //(arg1 + (arg2 + arg3))
}

那么如下调用：

```cpp
foldSumR(1, 2, 3)
```

就等价于

```cpp
(1 + (2 + 3))
```

这意味着下面的例子不再能通过编译：

```cpp
std::cout << flodSumR(std::string("hello"), "world", "!") << '\n';  //错误
```

然而下面的例子现在可以通过编译了：

```cpp
std::cout << foldSumR("hello", "world", std::string("!")) << '\n';  //OK
```

因为在几乎所有情况下都是从左到右求值，所以通常推荐使用左折叠的方式来将参数包展开到尾部（除非这样写会导致错误）：

```cpp
(... + args)    //推荐的折叠表达式写法
```

### 10.2.1 处理空参数包

如果折叠表达式被用于空参数包，那么下面的规则将会生效：

* 如果使用&&运算符，结果将会是true
* 如果使用||运算符，结果将会是false
* 如果使用,运算符，结果将会是void()
* 对所有其他运算符，调用是错误的

对于所有其他情况（任何情况下也都该如此）你可以添加一个初始值：给定一个参数包`args`，一个初始值`value`，一个操作符`op`，C++17允许我们这么写:

* 要么是一个**二元左折叠表达式**：

```cpp
(value op ... op args)
```

将会被扩展为：*(((value **op** arg1) **op** arg2) **op** arg3) **op*** ...

*要么是一个**二元右折叠表达式**：

```cpp
(args op ... op value)
```

将会被扩展为：*arg1 **op** (arg2 **op** ... (argN **op** value))*

省略号两侧的操作符`op`必须是相同的。

例如，下面的定义允许我们传递一个空参数包：

```cpp
template<typename... T>
auto foldSum (T... s) {
    return (0 + ... + s);   //即便sizeof...(s)==0也能正常工作
}
```

概念上讲，我们把0加在开始或者最后应该没有区别：

```cpp
template<typename... T>
auto foldSum (T... s) {
    return (s + ... + 0);   //即使sizeof...(s)==0也能正常工作
}
```

但是就像一元折叠表达式会有不同的求值顺序一样（见10.2节），二元折叠表达式也推荐使用左折叠的方式：

```cpp
(val + ... + args);     //二元折叠表达式的推荐写法
```

另外，第一个操作数可能是特殊的，就像这个例子一样：

```cpp
template<typename... T>
void print (const T&... args)
{
    (std::cout << ... << args) << '\n';
}
```

在这里，输出第一个传入的参数之后将仍然会返回输出流，然后其他参数再以相同的方式输出。其他的写法可能都通不过编译甚至可能做一些奇怪的事。例如，下面的：

```cpp
std::cout << (args << ... << '\n');
```

一个类似`print(1)`的调用将能够编译但会打印出1左移'\n'位的值，'\n'的值一般是10，所以打印出的结果就是1024。

注意这里的`print()`示例中所有打印出的参数之间没有空白分割符。一个像`print("hello", 42, "world")`的调用将会打印出：

```cpp
hello42world
```

为了用空格分隔每个元素，你可以使用一个辅助函数来确保除了第一个元素之外的每个元素之前都会填充一个空格。这可以像下面的`spaceBefore()`函数一样：

*tmpl/addspace.hpp*

```cpp
template<typename T>
const T& spaceBefore(const T& arg) {
    std::cout << ' ';
    return arg;
}

template<typename First, typename... Args>
void print (const First& firstarg, const Args&... args) {
    std::cout << firstarg;
    (std::cout << ... << spaceBefore(args)) << '\n';
}
```

这里，

```cpp
(std::cout << ... << spaceBefore(args))
```

被展开为:

```cpp
std::cout << spaceBefore(arg1) << spaceBefore(arg2) << ...
```

因此，对于参数包中`args`中的参数都会调用一次辅助函数来打印出一个前置空格。为了确保不会在第一个参数之前也打印一个空格，我们特意加了第一个不用`spaceBefore()`的参数。

注意参数包的正确输出要求当一个`spaceBefore()`函数调用时它前边的所有输出都已经完成。得益于有定义的表达式求值顺序（见7.2节），这段代码自从C++17起将能够保证正确工作。

我们也可以使用一个lambda在`print()`函数内定义一个`spaceBefore()`：

```cpp
template<typename First, typename... Args>
void print (const First& firstarg, const Args&... args) {
    std::cout << firstarg;
    auto spaceBefore = [](const auto& arg) {
        std::cout << ' ';
        return arg;
    };
    (std::cout << ... << spaceBefore(args)) << '\n';
}
```

然而，注意lambda默认以值传递返回值，这意味着这将创建参数的拷贝。避免这种情况的方法就是显式把lambda的返回类型声明为`const auto&`或者 `decltype(auto)`：

```cpp
template<typename First, typename... Args>
void print (const First& firstarg, const Args&... args) {
    std::cout << firstarg;
    auto spaceBefore = [](const auto& arg) -> const auto& {
        std::cout << ' ';
        return arg;
    };
    (std::cout << ... << spaceBefore(args)) << '\n';
}
```

如果你不把这些合在一个语句中那么C++就不再是C++：

```cpp
template<typename First, typename... Args>
void print (const First& firstarg, const Args&... args) {
    std::cout << firstarg;
    (std::cout << ... << [](const auto& arg) -> decltype(auto) {
        std::cout << ' ';
        return arg;
    }(args)) << '\n';
}
```

然而，`print()`的一个更简单的实现方式是使用lambda同时打印出空格和参数：

```cpp
template<typename First, typename... Args>
void print(First first, const Args&... args) {
    std::cout << first;
    auto outWithSpace = [](const auto& arg) {
                            std::cout << ' ' << arg;
                        };
    (..., outWithSpace(args));
    std::cout << '\n';
}
```

通过添加一个额外的用`auto`声明的模板参数(见12.1.1节)我们可以使`print()`变得更加灵活，可以通过参数来控制分割符是一个字符，或是一个字符串或是任何其它可打印的类型。

### 10.2.2 支持的运算符

你可以在折叠表达式中使用除了. -> []之外的二元运算符。

#### 折叠函数调用

折叠表达式也可以被用于逗号运算符来将多条语句转换为一条。例如，你可以折叠逗号运算符，来对一个基类数量可变的类的每个基类都调用一次成员函数：

*tmpl/foldcalls.cpp*

```cpp
#include <iostream>

//可变参数基类的模板
template<typename... Bases>
class MultiBase : private Bases...
{
  public:
    void print() {
        //调用每一个基类的print()函数
        (... , Bases::print());
    }
};

struct A {
    void print() {std::cout << "A::print()\n";}
};

struct B {
    void print() {std::cout << "B::print()\n";}
};

struct C {
    void print() {std::cout << "C::print()\n";}
};

int main()
{
    MultiBase<A, B, C> mb;
    mb.print();
}
```

在这里，

```cpp
template<typename... Bases>
class MultiBase : private Bases...
{
    ...
};
```

允许我们用可变数量的基类来实例化对象：

```cpp
MultiBase<A, B, C> mb;
```

而且

```cpp
(... , Bases::print());
```

这句折叠表达式会被展开成调用每一个基类的`print`函数。也就是说，这个语句会被展开为如下形式：

```cpp
(A::print(), B::print()), C::print();
```

然而，请注意因为逗号运算符本身的特性无论我们使用左折叠还是右折叠都没有什么区别，结果都是所有函数从左向右调用。如果写成

```cpp
(Bases::print(), ...);
```

括号将会将一个`print()`调用和另外两个`print()`调用的结果连接起来，就像下面这样：

```cpp
A::print(), (B::print(), C::print());
```

但是因为逗号表达式被定义的求值顺序总是逗号左边的先求值，然后逗号右边的再求值，所以即使是展开成这种形式也会从左向右进行调用。

不过，因为左折叠更符合正确的求值顺序，因此当使用逗号表达式调用多次函数时推荐使用左折叠表达式。

#### 合并哈希函数

另一个使用逗号运算符的例子是合并哈希值。这可以像下面这样做：

```cpp
template<typename T>
void hashCombine (std::size_t& seed, const T& val)
{
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template<typename... Types>
std::size_t combinedHashValue (const Types&... args)
{
    std::size_t seed = 0;   //初始化seed
    (... , hashCombine(seed, args));    //hashCombine()调用链
    return seed;
}
```

通过调用

```cpp
std::size_t combinedHashValue ("Hello", "World", 42);
```

折叠表达式将被展开为

```cpp
(hashCombine(seed, "Hello"), hashCombine(seed, "World")), hashCombine(seed, 42);
```

通过这个定义我们可以轻易的为一个新的类型例如`Customer`定义出一个新的哈希函数对象：

```cpp
struct CustomerHash
{
    std::size_t operator() (const Customer& c) const {
        return combinedHashValue(c.getFirstname(), c.getLastname(), c.getValue());
    }
};
```

这样我们就可以将`Customers`放在无序容器里了：

```cpp
std::unordered_set<Customer, CustomerHash> coll;
```

#### 折叠的路径遍历

你可以使用折叠表达式在一颗二叉树中使用->*遍历路径：

*templ/foldtraverse.cpp*

```cpp
//定义二叉树结构和遍历辅助函数
struct Node {
    int value;
    Node* left;
    Node* right;
    Node(int i=0) : value(i), left(nullptr), right(nullptr) {
    }
    ...
};
auto left = &Node::left;
auto right = &Node::right;

//使用折叠表达式遍历树
template<typename T, typename... TP>
Node* traverse (T np, TP... paths) {
    return (np ->* ... ->* paths);  //np ->* path1 ->* path2...
}

int main()
{
    //初始化二叉树结构
    Node* root = new Node{0};
    root->left = new Node{1};
    root->left->right = new Node{2};
    ...
    //遍历二叉树:
    Node* node = traverse(root, left, right);
    ...
}
```

这里，

```cpp
(np ->* ... ->* paths)
```

使用了折叠表达式来遍历从`np`开始的可变长度的路径。当调用：

```cpp
traverse(root, left, right);
```

时折叠表达式会展开为：

```cpp
root -> left -> right
```

### 10.2.3 为类型使用折叠表达式

通过使用类型特征我们也可以用折叠表达式来处理模板参数包（作为莫欧版参数被传入的任意数量的类型）。例如，你可以使用一个折叠表达式来判断传递的一列值是否是同一类型的：

*tmpl/ishomogeneous.hpp*

```cpp
//检查传入的参数是否是同一类型
template<typename T1, typename... TN>
struct IsHomogeneous {
    static constexpr bool value = (std::is_same<T1, TN>::value && ...);
};

//检查传递的参数是否是同一类型
template<typename T1, typename... TN>
constexpr bool isHomogeneous(T1, TN...)
{
    return (std::is_same<T1, TN>::value && ...);
}
```

类型特征`IsHomegeneous<>`可以像下面这样使用：

```cpp
IsHomogeneous<int, Size, decltype(42)>::value
```

在这个例子中折叠表达式会被展开为：

```cpp
std::is_same<int, MyType>::value && std::is_same<int, decltype(42)>::value
```

函数模板`isHomogeneous<>()`可以像下面这样使用：

```cpp
isHomogeneous(43, -1, "hello", nullptr)
```

在这个例子中折叠表达式会被展开为：

```cpp
std::is_same<int, int>::value && (std::is_same<int, const char*>::value && std::is_same<int, std::nullptr_t>::value)
```

像通常一样，&&运算符仍然是短路求值（会在上式中第一个为false的项处中断）。

标准库中的`std::array<>`的推导指引就使用了折叠表达式特性（见8.2.6节）

## 10.3 后记

折叠表达式最早由Andrew Sutton和Richard Smith在[https://wg21.link/n4191](https://wg21.link/n4191)上提出。最终被接受的正式提案由Andrew Sutton和Richard Smith在[https://wg21.link/n4295](https://wg21.link/n4295)上发表。对*，+，&，|等运算符的空序列支持随后根据Thibaut Le Jehan在[https://wg21.link/p0036](https://wg21.link/p0036)上的提议而被删除。

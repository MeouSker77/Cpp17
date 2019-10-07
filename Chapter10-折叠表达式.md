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

这样的实现不仅写起来很笨重，还会给C++编译期带来很大压力。通过写成

```cpp
template<typename... T>
auto foldSum (T... args) {
    return (... + args);    //arg1 + arg2 + arg3 ...
}
```

无论对程序员还是编译器来说效果都更好。

## 10.2 使用折叠表达式

给定一个参数args和一个操作符op，C++17允许我们写：

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

但是要记住只有当两边有一边是std::string时才能使用+。因为此处使用了左折叠，因此调用等价于

```cpp
(std::string("hello") + "world") + "!"
```

第一个括号会返回一个std::string，所以能够继续和"!"进行+运算。

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

对于所有其他情况（任何情况下也都该如此）你可以添加一个初始值：给定一个参数包args，一个初始值value，一个操作符op，C++17允许我们这么写:

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

省略号两侧的操作符*op*必须是相同的。

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

一个类似print(1)的调用将能够编译但会打印出1左移'\n'位的值，'\n'的值一般是10，所以打印出的结果就是1024。

注意这里的print()示例中所有打印出的参数之间没有空白分割符。一个像print("hello", 42, "world")的调用将会打印出：

```cpp
hello42world
```

为了用空格分隔每个元素，你可以使用一个辅助函数来确保除了第一个元素之外的每个元素之前都会填充一个空格。这可以像下面的spaceBefore()函数一样：

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

因此，对于参数包中args中的参数都会调用一次辅助函数来打印出一个前置空格。为了确保不会在第一个参数之前也打印一个空格，我们特意加了第一个不用spaceBefore()的参数。


注意参数包的正确输出要求当一个spaceBefore()函数调用时它前边的所有输出都已经完成。得益于有定义的表达式求值顺序（见7.2节），这段代码自从C++17起将能够保证正确工作。






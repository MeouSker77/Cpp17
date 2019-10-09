# Chapter11 使用字符串作为模板参数

一直以来每个C++版本都在逐渐放宽关于什么东西可以作为模板参数的标准。到了C++17标准再一次放宽。以前必须在当前作用域的外部定义这些实体，才能使用这些实体作为模板实参，但现在不再需要这样。

## 11.1 在模板中使用字符串

非类型的模板参数只能用于持久的（静态或全局的）整数类型（包括枚举），（对象/函数/成员的）指针，对象或函数的左值引用或者std::nullptr_t（nullptr的类型）。

对于指针类型，以前只有指向全局变量的指针才可以作为模板参数，这意味着你不能直接传递一个字符串字面量。然而，自从C++17，你可以使用指向静态变量的指针。例如：

```cpp
template<const char* str>
class Message {
    ...
};

extern const char hello[] = "Hello, World!";    //外部全局变量
const char hello11[] = "Hello, World!";         //内部全局变量

void foo()
{
    Message<hello> msg;         //OK(任何C++版本)
    Message<hello11> msg11;     //OK自从C++11

    static const char hello17[] = "Hello World!";   //静态变量
    Message<hello17> msg17;     //OK自从C++17
}
```

也就是说，自从C++17你仍然需要两行来将一个字符串字面量传递给模板，但是你可以将第一行放在和模板实例化相同的作用域之中。

这种能力也解决了很多不幸的约束：自从C++11起你可以给一个类模板传递一个指针作为参数：

```cpp
template<int* p>
struct A {
};

//如下两行代码需写在全局作用域中，下同（译者注）
int num;
A<&num> a;      //OK since C++11
```

以前你不能使用返回地址的编译期函数，但是现在可以了：

```cpp
int num;
constexpr int* pNum() {
    return &num;
}
A<pNum()> b;    //C++17之前错误，之后正确
```

允许使用所有的持久变量作为非模板参数最早由Richard Smith在[https://wg21.link/n4198](https://wg21.link/n4198)上提出。最终被接受的正式提案由Richard Smith在[https://wg21.link/n4268](https://wg21.link/n4268)上发表。

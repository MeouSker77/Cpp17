# Chapter 2 带初始化的`if`和`switch`语句

`if`和`switch`结构现在允许我们在通常的条件表达式里声明一个初始化语句。例如，你可以这样写：

```cpp
if (status s = check(); s != status::success) {
    return s;
}
```

在这里，初始化语句：

```cpp
status s = check();
```

初始化了一个变量`s`，这个变量在整个`if`语句中都是可访问的。

## 2.1 带初始化的`if`语句

任何在`if`条件表达式中初始化的变量，都可以在整个`if-else`语句中访问。例如：

```cpp
if (std::ofstream strm = getLogStrm(); coll.empty()) {
    strm << "<no data>\n";
}
else {
    for (const auto& elem : coll) {
        strm << elem << '\n';
    }
}
//到此处strm不再有效
```

`strm`的析构函数会在`if-else`语句结束时调用。

另一个例子是关于锁的使用，假设我们要在并发的环境中执行一个依赖某些条件的任务：

```cpp
if (std::lock_guard<std::mutex> lg{collMutex}; !coll.empty()) {
    std::cout << coll.front() << '\n';
}
```

通过使用类模板参数类型推导（见第8章）我们也可以写成如下形式：

```cpp
if (std::lock_guard lg{collMutex}; !coll.empty()) {
    std::cout << coll.front() << '\n';
}
```

在任何情况下，上面的代码等价于：

```cpp
{
    std::lock_guard<std::mutex> lg{collMutex};
    if (!coll.empty()) {
        std::cout << coll.front() << '\n';
    }
}
```

有一点很小的区别是上边的情况中`lg`被定义在`if`语句的作用域中，因此和条件语句在同一个作用域中，类似于在`for`循环中初始化一样。

注意任何在`if`语句中初始化的变量必须有一个变量名。否则初始化语句会在创建一个变量之后迅速销毁它。例如，初始化一个匿名的`lock_guard`将不会有效：

```cpp
if (std::lock_guard<std::mutex>{collMutex}; //运行时错误:
        !coll.empty()) {                    //此处相当于没有锁
    std::cout << coll.front() << '\n';      //此处相当于没有锁
}
```

原则上讲，一个`_`作为变量名就已经足够了：

```cpp
if (std::lock_guard<std::mutex> _{collMutes};   //OK,但是...
        !coll.empty()) {
    std::cout << coll.front() << '\n';
}
```

作为第三个例子，考虑向一个`map`或`unordered_map`中插入一个新的元素。你可以像下面这样检查插入是否成功：

```cpp
std::map<std::string, int> coll;
...
if (auto [pos,ok] = coll.insert({"new", 42}); !ok) {
    //如果插入失败，用pos迭代器处理错误:
    const auto& [key,val] = *pos;
    std::cout << "already there: " << key << '\n';
}
```

在这里我们又使用了结构化绑定（见第一章），来给两个返回值分别赋予一个有意义的名字，而不是用`first`和`second`成员来访问。在C++17之前，同样的检查必须写成如下的代码：

```cpp
auto ret = coll.insert({"new", 42});
if (!ret.second) {
    //如果插入失败，用ret.first处理错误
    const auto& elem = *(ret.first);
    std::cout << "already there: " << elem.first << '\n';
}
```

注意新的初始化`if`语句特性也适用于新的编译期`if`语句（见第9章）。

## 2.2 带初始化的`switch`语句

通过使用带初始化的`switch`语句，我们可以在`switch`语句中的条件表达式之前初始化一个对象或实体来决定控制流。

例如，我们可以初始化一个文件系统路径（见第19章），然后根据该路径的类型做不同的处理：

```cpp
using namespace std::filesystem;
...
switch(path p(name); status(p).type()) {
    case file_type::not_found:
        std::cout << p << "not found\n";
        break;
    case file_type::directory:
        std::cout << p << ":\n";
        for (auto& e : std::filesystem::directory_iterator(p)) {
            std::cout << "- " << e.path() << '\n';
        }
        break;
    default:
        std::cout << p << " exists\n";
        break;
}
```

在这里，初始化的路径`p`可以在整个`switch`语句中使用。

## 2.3 后记

带初始化的`if`和`switch`语句最早由Thomas Koppe在[https://wg21.link/p0305r0](https://wg21.link/p0305r0)中提出，最初目的是为了扩展`if`语句。该特性最终被接受的正式提案由Thomas Koppe在[https://wg21.link/p0305r1](https://wg21.link/p0305r1)发表。

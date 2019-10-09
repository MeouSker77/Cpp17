# Chapter 8 类模板参数推导

在C++17之前，你必须为一个类模板显式指明所有模板参数。例如，你可以省略下面语句中的`double`：

```cpp
std::complex<double> c{5.1,3.3};
```

或者省略下面语句中第二次指明的`std::mutex`：

```cpp
std::mutex mx;
std::lock_guard<std::mutex> lg(mx);
```

自从C++17，你需要指明类模板参数的要求被放宽了。你可以略过显式声明模板参数类型，只要构造函数能够推导出所有的模板参数。例如：

*你可以像下面这样声明：

```cpp
std::complex c{5.1,3.3};
```

*你可以像下面这样写：

```cpp
std::muetx mx;
std::lock_guard lg(mx);
```

## 8.1 使用类模板参数推导

只要传递给构造函数的参数可以用来推导出所有的模板参数就可以使用类模板参数推导。该推导支持所有方式的初始化（当然要保证初始化是有效的）：

```cpp
std::complex c1(1.1, 2.2);  //推导出std::complex<double>
std::complex c2{2.2, 3.3};  //推导出std::complex<double>
std::complex c3 = 3.3;      //推导出std::complex<double>
std::complex c4 = {4.4};    //推导出std::complex<double>
```

`c3`和`c4`的初始化方式也是可行的，因为你可以只用一个参数初始化一个`std::complex<>`，这已经足够推导出模板参数`T`的类型，之后会将`T`用于复数的实部和虚部部分：

```cpp
namespace std {
    template<typename T>
    class complex {
        constexpr complex(const T& re = T(), const T& im = T());
        ...
    }
};
```

然而，请注意模板参数必须能无歧义的推导出来。因此，下面的初始化语句不能正常工作：

```cpp
std::complex c5{5,3.3};     //错误：企图将T同时用于int和double
```

像通常一样推导模板参数时没有类型转换。

类模板参数推导也适用于可变参数模板的推导。例如，对于一个如下定义的`std::tuple<>`：

```cpp
namespace std {
    template<typename... Types>
    class tuple;
};
```

如下声明：

```cpp
std::tuple t{42, 'x', nullptr};
```

将会推导出`t`的类型为`std::tuple<int, char, std::nullptr_t>`。

你也可以推导无类型的模板参数。例如，我们可以像下面一样通过传递一个数组来同时推导出表示元素类型的数量的模板参数：

```cpp
template<typename T, int SZ>
class MyClass {
  public:
    MyClass (T(&)[SZ]) {
        ...
    }
};

MyClass mc("hello"); //推导出T是const char，SZ是6
```

这里我们推导出了`SZ`为6因为传递的参数是一个有6个字符的字符串字面量。

你甚至可以推导出lambda的类型或者推导自动类型模板参数（见12.1节）。

### 8.1.1 默认以拷贝方式推导

如果类模板参数可以用拷贝初始化的方式推导出来，那么会优先用这种方式。例如，在用一个元素初始化一个`std::vector`之后：

```cpp
std::vector v1{42};     //带有一个元素的vector<int>
```

使用这个`vector`来初始化另一个`vector`会被解释为创建一个拷贝：

```cpp
std::vector v2{v1};     //v2也是一个vector<int>
```

而不是假设中的一个以`vector`为元素的`vector`（即`vector<vector<int>>`)。

另外，这条规则适用于所有有效的初始化形式：

```cpp
std::vector v3(v1);         //v3也是vector<int>
std::vector v4 = {v1};      //v4也是vector<int>
auto v5 = std::vector{v1};  //v5也是vector<int>
```

只有当传递了多个参数以至于不能解释为一个拷贝的时候，才会把初值列的元素解释为元素类型为`vector`的`vector`：

```cpp
std::vector vv{v, v};       //vv是vector<vector<int>>
```

这就给可变参数模板的类型推导带来了问题：

```cpp
template<typename... Args>
auto make_vector(const Args&... elems) {
    return std::vector{elems...};
}

std::vector<int> v{1, 2, 3};
auto x1 = make_vector(v, v);    //vector<vector<int>>
auto x2 = make_vector(v);       //vector<int>还是vector<vector<int>>?
```

目前来看，编译器会自行处理这个问题，解决方法还在讨论之中。

### 8.1.2 推导lambda的类型

有了模板参数推导，我们第一次可以使用lambda（更精确地说：是lambda转换成的闭包类型）来实例化一个模板类。例如，我们可以提供一个泛型类，对一个任意的回调函数的调用进行包装和计数：

*tmpl/classarglambda.hpp*

```cpp
#include <utility>  //for std::forward()

template<typename CB>
class CountCalls
{
  private:
    CB callback;    //回调函数
    long calls = 0; //记录调用次数
  public:
    CountCalls(CB cb) : callback(cb) {
    }
    template<typename... Args>
    auto operator() (Args&&... args) {
        ++calls;
        return callback(std::forward<Args>(args)...);
    }
    long count() const {
        return calls;
    }
};
```

这里构造函数对`callback`函数进行封装，并将`CB`推导为它的类型。例如，我们可以通过传递一个lambda作为参数来初始化一个`CountCalls`对象：

```cpp
CountCalls sc([](auto x, auto y) {
                return x > y;
            });
```

这意味着排序准则`sc`的类型被推导为`CountCalls<TypeOfLambda>`。通过这种方式，我们可以统计被传递的排序准则被调用的次数：

```cpp
std::sort(v.begin(), v.end(), std::ref(sc));
std::cout << "sorted with " << sc.count() << " calls\n";
```

这里，被包装的lambda被用作排序准则，注意这里必须使用引用方式传递。因为如果不这么做那么`std::sort()`将会使用`sc`的一份拷贝作为排序准则，那么递增的计数器将是`sc`的拷贝中的计数器。

然而，我们可以给`std::for_each()`传递一个包装的lambda表达式。因为这个算法最后会返回被传入的函数，因此我们可以利用它的返回值的状态：

```cpp
auto fo = std::for_each(v.begin(), v.end(),
                        CountCalls([](auto i) {
                            std::cout << "elems: " << i << '\n';
                        }));
std::cout << "output with " << fo.count() << " calls\n";
```

### 8.1.3 没有部分类模板参数推导

注意，不像函数模板，类模板参数不能只推导一部分（即显式指明一部分模板参数类型）。例如：

```cpp
template<typename T1, typename T2, typename T3 = T2>
class C
{
  public:
    C (T1 x = T1{}, T2 y = T2{}, T3 z = T3{}) {
        ...
    }
    ...
};

//推导全部参数：
C c1(22, 44.3, "hi");       //OK: T1是int，T2是double，T3是const char*
C c2(22, 44.3);             //OK: T1是int，T2和T3是double
C c3("hi", "guy");          //OK: T1，T2和T3是const char*

//推导部分参数：
C<string> c4("hi", "my");   //错误：只有T1显式定义
C<> c5(22, 44.3);           //错误：T1和T2都没有显式定义
C<> c6(22, 44.3, 42);       //错误：T1和T2都没有显式定义

//指明全部参数：
C<string, string, int> c7;      //OK: T1和T2是string，T3是int
C<int, string> c8(52, "my");    //OK: T1是int，T2和T3是string
C<string, string> c9("a", "b", "c");    //OK: T1,T2和T3是string
```

注意第三个模板参数有一个默认值。出于这个原因，如果第二个参数已经指明了那么就没必要再指明第三个参数。

如果你想知道为什么不支持部分模板参数推导，这里有一个导致这个决定的例子：

```cpp
std::tuple<int> t(42, 43);      //仍然是错误的
```

`std::tuple`是一个可变参数模板，你可以指明任意数量的参数。因此，在上边的例子中会不清楚它是无意中错误的初始化了只有一个参数的`tuple`还是故意要这么写。这很明显是有问题的，指明部分参数类型的推导以后仍然有可能被添加到C++标准中，不过要花费更多的时间来思考这种问题。

不幸的是，缺少推导部分参数模板的能力意味着一些新的问题。我们仍然不能轻易的使用lambda指明一个关联容器的排序准则或者无序容器的哈希函数：

```cpp
std::set<Cust> coll([](const Cust& x, const Cust& y) {   //仍然是错误的
                            return x.name() > y.name();
});
```

我们仍然必须指明lambda的类型，因此我们需要像下面这么写：

```cpp
auto sortcrit = [](const Cust& x, const Cust& y) {
                    return x.name() > y.name();
};
std::set<Cust, decltype(sortcrit)> coll(sortcrit);  //OK
```

### 8.1.4 使用类模板参数推导代替一些便利的函数

通过使用类模板参数推导，原则上讲我们可以避免使用几个通过传递的参数来推断类型的便利函数。一个明显的例子是`make_pair()`，它能让我们避免指明参数的类型。例如：在

```cpp
std::vector<int> v;
```

之后，我们可以使用

```cpp
auto p = std::make_pair(v.begin(), v.end());
```

来代替:

```cpp
std::pair<typename std::vector<int>::iterator,
          typename std::vector<int>::iterator> p(v.begin(), v.end());
```

然而，这里的`make_pair()`已经不再需要了，我们可以像下面这样简单的声明：

```cpp
std::pair p(v.begin(), v.end());
```

然而，`std::make_pair()`是一个很好的例子来展示有时候便利函数比模板参数推导做了更多的事。事实上，`std::make_pair()`也是退化的，也就意味着传递的字符串字面量的类型会被转换为`const char*`：

```cpp
auto q = std::make_pair("hi", "world");     //指针组成的pair
```

在这个例子中，`p`的类型为`std::pair<const char*, const char>`。

通过使用类模板参数推导，事情变得更加复杂。让我们看一下下面类似于`std::pair`的一个类的部分声明：

```cpp
template<typename T1, typename T2>
struct Pair1 {
    T1 first;
    T2 first;
    Pair1(const T1& x, const T2& y) : first{x}, second{y} {}
};
```

关键点在于参数是以引用方式传递进来的。根据语言规则，当传递进的模板参数类型为引用时，参数类型不应该退化（将原生数组转换为原始指针这一行为的术语）。因此，当出现下面的情况时：

```cpp
Pair1 p1{"hi", "world"};    //推导为大小不同的两个数组组成的pair，但是。。。
```

`T1`被推导为`char[3]`，`T2`被推导为`char[6]`。原则上讲，这样的推导是有效的。
然而，我们使用了`T1`和`T2`来声明成员`first`和`second`。因此，它们会被声明为

```cpp
char first[3];
char second[6];
```

然而从一个数组的右值来初始化一个数组是不允许的。这就像企图编译下面的语句：

```cpp
const char x[3] = "hi";
const char y[6] = "world";
char first[3] {x};      //错误
char second[6] {y};     //错误
```

注意当我们以值的方式传递参数时就不会再有这个问题了：

```cpp
template<typename T1, typename T2>
struct Pair2 {
    T1 first;
    T2 second;
    Pair2(T1 x, T2 y) : first{x}, second{y} {}
}
```

如果我们写出下面的语句：

```cpp
Pair2 p2{"hi", "world"};    //指针组成的pair
```

`T1`和`T2`都会推导为`const char *`。

因为`std::pair<>`类的构造函数被声明为以引用方式传递参数，所以你可能觉得下面的初始化语句不能通过编译：

```cpp
std::pair p{"hi", "world"}; //看起来推导为大小不同的数组组成的pair，但是。。。
```

但事实上它能通过编译。原因是我们使用了推导指引。

## 8.2 推导指引

你可以定义一个特定的推导指引来提供附加的推导。例如，你可以定义任何时候推导`Pair3`的类型时，都假设以值传递参数的情况来进行推导。

```cpp
template<typename T1, typename T2>
struct Pair3 {
    T1 first;
    T2 second;
    Pair3(const T1& x, const T2& y) : first{x}, second{y} {}
};

//构造函数的推导指引
template<typename T1, typename T2>
Pair3(T1, T2) -> Pair3<T1, T2>;
```

这里->的左边是我们想要推导的。在这个例子中，是使用两个传值的参数来创建的`Pair3`。在->的右边我们定义了推导结果。在这个例子中，`Pair3`使用`T1`和`T2`类型实例化。你可能会认为这和构造函数做的事情一样。然而，构造函数以引用传递参数，因此传递进来的数组或字符串字面量不会退化。因为这个推导指引，模板参数会发生退化，也就意味着传递进来的数组或字符串字面量会退化到相应的指针类型。

例如，当有如下声明时：

```cpp
Pair3 p3{"hi", "world"};    //推导出指针的pair
```

推导指引会推导出`T1`和`T2`的类型为`const char *`即便原本的字面量的类型为`const char [3]`和`const char [6]`。这个声明的效果就像下面一样：

```cpp
Pair3<const char*, const char*> p3{"hi", "world"};
```

构造函数仍然以引用方式传递参数。推导指引只影响模板类型的推导，它和`T1`和`T2`被推导出之后构造函数的实际调用没有任何关系。

### 8.2.1 使用推导指引来强行退化

就像上一个示例展示的那样，这些规则的一个应用就是确保模板参数`T`在应该退化的时候发生退化。考虑下面的经典类模板：

```cpp
template<typename T>
struct C {
    C(const T&) {
    }
    ...
}
```

如果我们传递一个字符串字面量"hello"，`T`会被推导为字面量的类型，即`const char [6]`:

```cpp
C x{"hello"};   //T被推导为const char [6]
```

原因是当参数以引用方式传递时模板参数不会退化为相应的指针类型。

只需要一个简单的推导指引：

```cpp
template<typename T> C(T) -> C<T>;
```

我们就能解决这个问题：

```cpp
C x{"hello"};   //T被推导为const char *
```

现在，因为推导指引以值传参，所以"hello"的类型退化为`const char *`。

出于这些原因，一个相应的推导指引对很多构造函数以引用传参的模板类是很有必要的。

### 8.2.2 非模板推导指引

推导指引不一定必须是模板也不需要显式的调用构造函数。例如，像下面的结构体和推导指引：

```cpp
template<typename T>
struct S {
    T val;
};

S(const char *) -> S<std::string>;   //用字符串字面量初始化时将推导为S<std::string>
```

下面的声明是可行的，因为以`const char*`初始化时会被推导为`std::string`类型，所以传递进入的字符串字面量会被隐式转换为`std::string`：

```cpp
S s1{"hell"};       //OK,等同于S<std::string> s1{"hell"};
S s2 = {"hello"};   //OK,等同于S<std::string> s2 = {"hello"};
S s3 = S{"hello"};  //OK,两个S都被推导为S<std::string>
```

注意聚合体需要列表初始化（下面示例中推导仍然有效，但不能初始化）：

```cpp
S s4 = "hello";     //错误，不能用这种方式初始化聚合体
```

### 8.2.3 推导指引与构造函数冲突

推导指引会和类的构造函数冲突。类模板参数推导以模板指引为最高优先级。如果一个构造函数和一个推导指引都能和参数匹配，那么推导指引将会胜出。考虑我们有如下定义：

```cpp
template<typename T>
struct C1 {
    C1(const T&) {}
};
C1(int) -> C1<long>;
```

当传递一个`int`作为参数时，将会使用推导指引，因为它是一个更好的匹配（因为它不是模板而是精确匹配）。因此，`T`会被推导为`long`：

```cpp
C1 x1(42);      //T被推导为long
```

但是如果我们传一个`char`，构造函数将是一个更好的匹配（因为它不需要类型转换），因此会推导`T`的类型为`char`：

```cpp
C1 x3('x');     //T被推导为char
```

因为以值传递和以引用传递的匹配程度是相同的而且推导指引有更高的优先度，所以通常将推导指引定义为以值传参是很好的选择（这样做还有可以自动进行退化的优点（见8.2.1节））。

### 8.2.4 `explicit`推导指引

一个推导指引可以被声明为`explicit`。这样在`explicit`禁止类型转换的时候这条指引就会被忽略。例如，给出如下定义：

```cpp
template<typename T>
struct S {
    T val;
};

explicit S(const char*) -> S<std::string>;
```

此时一个拷贝初始化（使用=）传递推导指引参数时将会忽略`explicit`的推导指引
。在这里，这就意味着这种初始化方式是无效的：

```cpp
S s1 = {"hello"};       //错误（推导指引被忽略或者说无效）
```

直接初始化或者在=右侧有一个明确的推导仍然可行：

```cpp
S s2{"hello"};          //OK，等同于S<std::string> s1{"hello"};
S s3 = S{"hello"};      //OK
S s4 = {S{"hello"}};    //OK
```

作为另一个例子，我们可以这样做：

```cpp
template<typename T>
struct Ptr
{
    Ptr(T) { std::cout << "Ptr(T)\n"; }
    template<typename U>
    Ptr(U) { std::cout << "Ptr(U)\n"; }
};

template<typename T>
explicit Ptr(T) -> Ptr<T*>;
```

效果如下：

```cpp
Ptr p1{42};     //根据推导指引推导出Ptr<int*>
Ptr p2 = 42;    //根据构造函数推导出Ptr<int>
int i = 42;
Ptr p3{&i};     //根据推导指引推导出Ptr<int**>
Ptr p4 = &i;    //根据构造函数推导出Ptr<int*>
```

### 8.2.5 聚合体的推导指引

模板推导可以用于泛型聚合体来允许模板参数推导。例如对于：

```cpp
template<typenaem T>
struct A {
    T val;
};
```

在没有推导指引的情况下任何形式的模板参数推导都是错误的：

```cpp
A i1{42};       //错误
A s1("hi");     //错误
A s2{"hi"};     //错误
A s3 = "hi";    //错误
A s4 = {"hi"};  //错误
```

你必须指明类型`T`：

```cpp
A<int> i2{42};
A<std::string> s5 = {"hi"};
```

但如果有了这样的一个推导指引:

```cpp
A(const char*) -> A<std::string>;
```

你可以像下面这样初始化聚合体：

```cpp
A s2{"hi"};     //OK
A s4 = {"hi"};  //OK
```

然而，像通常的聚合体初始化一样，你需要使用花括号。否则，虽然类型`T`被成功的推导了出来，但初始化将会产生错误：

```cpp
A s1("hi");     //错误：T是string，但聚合体不能初始化
A s3 = "hi";    //错误：T是string，但聚合体不能初始化
```

`std::array`的推导指引（见8.2.6节）是另一个聚合体的推导指引的例子。

### 8.2.6 标准推导指引

C++标准库也引入了一些推导指引。

#### 迭代器推导

例如，为了能够从定义一个初始化区间的迭代器的类型推导出元素的类型，容器们现在有了一个像下面`std::vector<>`一样的推导指引：

```cpp
//让std::vector<>通过初始化迭代器来推断元素类型：
namespace std {
    template<typename Iterator>
    vector(Iterator, Iterator)
    -> vector<typename iterator_traits<Iterator>::value_type>;
}
```

其效果如下：

```cpp
std::set<float> s;
std::vector(s.begin(), s.end());    //OK,类型推断为std::vector<float>
```

#### `std::array<>`推导

一个更有趣的例子`std::array<>`能同时推导出元素类型和元素数量：

```cpp
std::array a{42, 45, 77};   //OK,类型推断为std::array<int, 3>
```

`std::array<>`定义了如下的推导指引：

```cpp
//让std::array<>能推导出元素数量（所有元素类型必须相同）
namespace std {
    template<typename T, typename... U>
    array(T, U...) ->
    array<enable_if_t<(is_same_v<T,U> && ...), T>,
    (1 + sizeof...(U))>;
}
```

这个推导指引使用了折叠表达式（见第10章）

```cpp
(is_same_v<T,U> && ...)
```

来确保传递进的所有参数的类型是相同的。因此，下面的写法是不可行的：

```cpp
std::array a{42, 45, 77.7}; //错误：元素类型不同
```

#### （无序）`map`推导

通过为关联容器（`map`，`multimap`，`unordered_map`，`unordered_multimap`）定义推导指引可以看出推导指引的复杂性。这些容器的元素类型为`std::pair<const keytype, valuetype>`。键的常量属性是必须的，因为元素的位置依赖于键的值，因此如果能修改键的话可能会导致容器出现错误。

因此，在C++17标准里的`std::map`:

```cpp
namespace std {
    template<typename Key, typename T,
             typename Compare = less<Key>,
             typename Alllocator = allocator<pair<const Key, T>>>
    class map {
        ...
    };
}
```

会为像下面的构造函数：

```cpp
map(initializer_list<pair<const Key, T>>,
    const Compare& = Compare(),
    const Allocator& = Allocator());
```

定义下面的推导指引：

```cpp
namespace std {
    template<typename Key, typename T,
             typename Compare = less<Key>,
             typename Allocator = allocator<pair<const Key, T>>>
    map(initializer_list<pair<const Key, T>>,
        Compare = Compare(),
        Allocator = Allocator())
        -> map<Key, T, Compare, Allocator>;
}
```

所有参数都以值传递，这个推导指引允许传递的comparator或者allocator进行退化（见8.2.1）。然而，我们天真的使用了和参数一样的类型，这意味着初值列的键需要是常量类型。因此，下面的代码就像Ville Voutilainen在[https://wg21.link/lwg3025](https://wg21.link/lwg3025)提出的一样不能正常工作：

```cpp
std::pair elem1{1,2};
std::pair elem2{3,4};
...
std::map m1{elem1, elem2};  //在上述的推导指引中是错误的
```

因为这里的元素类型被推导为`std::pair<int,int>`，这和推导指引中需要一个常量属性的键相冲突。因此，你仍然必须像下面这样写：

```cpp
std::map<int, int> m1{elem1, elem2};    //OK
```

因此，应该去掉推导指引中的`const`：

```cpp
namespace std {
    template<typename Key, typename T,
             typename Compare = less<Key>,
             typename Allocator = allocator<pair<const Key, T>>>
    map(initializer_list<pair<Key, T>>,
        Compare = Compare(),
        Allocator = Allocator())
        -> map<key, T, Compare, Allocator>
}
```

然而，此时如果初值列中的键为常量属性，那么将会调用构造函数进行推导（因为构造函数是更精确的匹配），但这样就会失去自动退化的能力。所以，我们需要为推导指引重载一个键为`const`时候的版本。

#### 智能指针没有推导指引

注意C++标准库的有些地方并没有推导指引。你可能会希望能有一个为`shared`和`unique`指针设计的推导指引，然后可以将：

```cpp
std::shared_ptr<int> sp{new int(7)};
```

写成：

```cpp
std::shared_ptr sp{new int(7)};     //不支持
```

这并不能正常工作，因为相应的构造函数是一个模板，所以所以没有隐式的推导指引：

```cpp
namespace std {
    template<typename T> class shared_ptr {
      public:
        ...
        template<typename Y> explicit shared_ptr(Y* p);
        ...
    };
}
```

`Y`是一个和`T`不同的模板参数因此我们并不能从构造函数中的`Y`的类型推导出`T`的类型。这么设计是为了实现像下面这样的特性：

```cpp
std::shared_ptr<Base> sp{new Derived(...)};
```

相应的推导指引很容易提供：

```cpp
namespace std{
    template<typename Y> shared_ptr(Y*) -> shared_ptr<Y>;
}
```

然而，这将意味着分配数组时这个推导指引也会被使用：

```cpp
std::shared_ptr sp{new int[10]};    //将会被推导为shared_ptr<int>
```

在C++中我们经常会遇到来自C语言的一个讨厌的问题，即某个对象的指针和这个对象的数组有相同的类型，或者退化后有相同的类型。因为这个问题太过危险，所以C++标准委员会决定不给智能指针定义推导指引。对于单个对象的指针，你仍然必须像下面这样调用：

```cpp
std::shared_ptr<int> sp1{new int};      //OK
auto sp2 = std::make_shared<int>();     //OK
```

对于数组，你需要这样：

```cpp
std::shared_ptr<std::string> p(new std::string[10],
                                [](std::string* p) {
                                    delete[] p;
                                });
```

或者

```cpp
std::shared_ptr<std::string> p(new std::string[10],
                                std::defalut_delete<std::string[]>());
```

## 8.3 后记

类模板参数推导最早由Michael Spertus于2007年在[https://wg21.link/n2332](https://wg21.link/n2332)提出。2013年Michael Spertus 和David Vandevoorde在[https://wg21.link/n3602](https://wg21.link/n3602)上再一次提出了这个提案。最终被接受的正式提案经Michael Spertus, Faisal Vali和Richard Smith在[https://wg21.link/p0512r0](https://wg21.link/p0512r0)上和Jason Merrill在[https://wg21.link/p0620r0](https://wg21.link/p0620r0)上和Michael Spertus和Jason Merrill在[https://wg21.link/p702r1](https://wg21.link/p702r1)上提出的修改之后由Michael Spertus, Faisal Vali和Richard Smith在[https://wg21.link/p0091r3](https://wg21.link/p0091r3)上提出。

标准库中对类模板参数推导支持由Michael Spertus, Walter E. Brown和Stephan T. Lavavej在[https://wg21.link/p0433r2](https://wg21.link/p0433r2)和[https://wg21.link/p0739r0](https://wg21.link/p0739r0)上添加。

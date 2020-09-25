# Chapter 19 文件系统库

一直到C++17`Boost.filesystem`终于被接纳进C++标准库。在这个过程中，为了适应新的语言特性，对这个库进行了很多调整。增加了更多与其他库之间的协调，同时清除了一些功能，也添加了一些缺失的功能。（例如，计算两个文件系统路径的相对路径）

## 19.1 基本的示例

让我们以一些基础的示例开始：

#### 打印出文件系统路径的属性

下面的程序让我们使用一个字符串作为文件系统路径并根据它的文件类型打印出一些信息：

*filesystem/checkpath.cpp*

```cpp
#include <iostream>
#include <filesystem>

int main (int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <path> \n";
        return EXIT_FAILURE;
    }

    std::filesystem::path p(argv[1]);   //p代表一个文件系统路径（可能不存在这个文件）
    if (!exists(p)) {               //路径p存在吗？
        std::cout << "path " << p << "does not exist\n";
    }
    else {
        if (is_regular_file(p)) {   //路径p是一个普通文件吗？
            std::cout << p << " exists with " << file_size(p) << " bytes\n";
        }
        else if (is_directory(p)) { //路径p是一个文件夹吗？
            std::cout << p << " is a directory containing:\n";
            for (auto& e : std::filesystem::directory_iterator(p)) {
                std::cout << "    " << e.path() << '\n';
            }
        }
        else {
            std::cout << p << " exists, but is not regular file or directory\n";
        }
    }
}
```

我们首先检查，传入的路径是否存在：

```cpp
std::filesystem::path p(argv[1]);       //p代表一个文件系统路径（路径可能不存在）
if (!exists(p)) {       //路径p存在吗？
    ...
}
```

如果存在，我们进行如下的检查：

* 如果它是一个普通文件，我们打印出它的大小：

```cpp
if (is_regular_file(p)) {       //p是一个普通文件吗？
    std::cout << p << " exists with " << file_size(p) << " bytes\n";
}
```

像下面这样调用程序：

```
checkpath checkpath.cpp
```

会像这样输出：

```

"./checkpath.cpp" exists with 897 bytes
```

注意路径的输出运算符会自动将路径用双引号括起来。

* 如果它是一个文件夹，我们就迭代里面的文件并打印出路径：

```cpp
if (is_directory(p)) {      //p是一个文件夹吗？
    std::cout << p << " is a directory containing:\n";
    for (auto& e : std::filesystem::directory_iterator(p)) {
        std::cout << "    " << e.path() << '\n';
    }
}
```

这里我们使用了`directory_iterator`，它提供了`begin()`和`end()`方法因此我们可以使用范围`for`语句遍历目录中的每一项。在这种情况下我们调用了每一个`directory_entry`的`path()`成员函数，它会返回条目的文件系统路径。

像下面这样调用程序：

```
checkpath .
```

会像这样输出：

```
"." is a directory containing:
    "./checkpath.cpp"
    "./checkpath.exe"
```

#### 创建不同类型的文件

下面的程序尝试在一个目录`/tmp`下创建不同类型的文件：

*filesystem/createfiles.cpp*

```cpp
#include <iostream>
#include <fstream>
#include <filesystem>

int main()
{
    namespace fs = std::filesystem;
    try {
        //创建/tmp/test:
        fs::path tmpDir("/tmp");
        fs::path testPath = tmpDir / "test";
        if (!create_directory(testPath)) {
            std::cout << "test directory already exists" << '\n';
        }

        //创建/tmp/test的链接:
        create_symlink(testPath, fs::path("testdir.lnk"));

        //创建数据文件/tmp/test/data.txt:
        testPath /= "data.txt";
        std::ofstream dataFile(testPath.string());
        if (dataFile) {
            dataFile << "this is my data\n";
        }

        //递归列出所有文件(也会遍历符号链接)
        auto allopt = fs::directory_options::follow_directory_symlink;
        for (auto& e : fs::recursive_directory_iterator(".", allopt)) {
            std::cout << e.path() << '\n';
        }
    }
    catch (fs::filesystem_error& e) {
        std::cerr << "exception: " << e.what() << '\n';
        std::cerr << "        path1: " << e.path1() << '\n';
    }
}
```

首先，我们将`fs`定义为命名空间`std::filesystem`的缩写：

```cpp
namespace fs = std::filesystem;
```

然后我们初始化了两个路径，使用操作符`/`来在目录`/tmp`里定义路径`/tmp/test`：

```cpp
fs::path tmpDir("/tmp");
fs::path testPath = tmpDir / "test";
```

然后，我们尝试在当前文件夹下创建指向`/tmp/test`的符号链接`testdir.lnk`:

```cpp
create_symlink(testPath, fs::path("testdir.lnk"));
```

然后我们创建了一个新的文件`/tmp/test/data.txt`，并写入了一些内容：

```cpp
testPath /= "data.txt";
std::ofstream dataFile(testPath.string());
if (dataFile) {
    dataFile << "this is my data\n";
}
```

这里我们使用运算符`/=`来扩展路径，并使用`string()`来将路径转换为字符串，并使用输出流打开这个字符串对应的文件。

最后，我们从当前目录递归的列出了所有文件：

```cpp
auto allopt = fs::directory_options::follow_directory_symlink;
for (auto& e : fs::recursive_directory_iterator(".", allopt)) {
    std::cout << "    " << e.path() << '\n';
}
```

因为我们使用了一个递归的目录迭代器并且允许遍历符号链接，所以我们应该有像下面的输出：

```
"./createfiles.cpp"
"./createfiles.exe"
"./testdir.lnk"
"./testdir.lnk/data.txt"
```

取决于不同的平台和权限我们在这里可能会遇到错误。对于那些没有被返回值覆盖到的情况，我们捕获相应的异常并打印出异常信息和它的第一个路径:

```cpp
try {
    ...
}
catch (fs::filesystem_error& e) {
    std::cerr << "exception: " << e.what() << '\n';
    std::cerr << "        path1: " << e.path1() << '\n';
}
```

例如，如果我们创建文件夹失败，将会打印出像这样的信息：

```
exception: filesystem error: cannot create directory: [/tmp/test]
           path1: "/tmp/test"
```

#### 根据文件类型进行不同的操作

下面的代码演示了怎么根据不同的文件系统类型或者不同的文件名进行不同的操作：

*filesystem/switchinit.cpp*

```cpp
#include <string>
#include <iostream>
#include <filesystem>
namespace std {
    using namespace std::experimental;
}

void checkFilename(const std::string& name)
{
    using namespace std::filesystem;

    switch (path p(name); status(p).type()) {
        case file_type::not_found:
            std::cout << p << " not found\n";
            break;

        case file_type::directory:
            std::cout << p << ":\n";
            for (auto& entry : std::filesystem::directory_iterator(p)) {
                std::cout << "- " << entry.path() << '\n';
            }
            break;

        default:
            std::cout << p << " exists\n";
            break;
    }
}

int main()
{
    for (auto name : {".", "ifinit.cpp", "nofile"}) {
        checkFilename(name);
        std::cout << '\n';
    }
}
```

这里我们使用了一个带初始化的`switch`语句（见2.2节）来获得文件类型：

```cpp
switch (path p(name); status(p).type()) {
    ...
}
```

`status(p)`创建了一个`file_status`，它的`type()`成员创建了一个`file_type`。这是故意被分为很多步操作，这样我们可以免去为自己不感兴趣的文件状态信息付出系统调用的代价。

## 19.2 原则和术语

在讨论文件系统库的细节之前我们必须先介绍一下设计原则和术语。这是必须的因为标准库要兼顾不同的操作系统并将它们映射到共同的API。

### 19.2.1 可移植性

C++标准不只标准化了所有可能的操作系统上的文件系统的共同之处。在很多情况下，它遵循POSIX标准，并且C++标准要求尽可能地遵循POSIX标准。只要一种行为有理由存在它就应该存在尽管可能有一些限制，对于可能出现的没有理由的行为，一个好的实现应该汇报一个错误。这些错误的可能示例有：

* 用于文件名的字符不被支持
* 文件系统元素不被支持（例如符号链接）

另外这些特定文件系统间的不同也可能导致问题：

* 大小写敏感一致性：
`"hello.txt"`，`"Hello.txt"`，`"hello.TXT"`要么指向同一个文件，要么指向三个文件。
* 绝对路径和相对路径：
在有一些系统上`"/bin"`是一个绝对路径（遵循POSIX的系统），然而在另一些系统上它不是（例如Windows）。

### 19.2.2 命名空间

文件系统库在`std`内有自己的子命名空间`filesystem`。为它引入一个缩写是很正常的操作：

```cpp
namespace fs = std::filesystem;
```

这允许我们使用`fs::current_path()`代替`std::filesystem::current_path()`。

之后的代码示例将使用`fs`作为相应的缩写。

### 19.2.3 `path`

文件系统库的关键是`path`。它代表文件系统中（可能）存在的一个文件的名字。它由一个可选的根名称，一个可选的根目录，和一系列通过路径分隔符分割的文件名组成。路径可以是相对的（此时文件的位置依赖于当前的工作路径）或绝对的。

它可能有不同的格式：

* 一个通用的格式，它是可移植的
* 一个本地格式，它是依赖于底层文件系统的

例如，通用格式`/tmp/test.txt`可能映射到本地的Windows格式`\tmp\test.txt`或者OpenVMS格式`[tmp]test.txt`。

特殊的文件名有：

* "."代表当前目录
* ".."代表父目录

通用的格式像下面这样：

```
    [rootname] [rootdir] [relativepath]
```

* 可选的根名称依赖于特定实现（例如，它可以是POSIX系统上的`//host`，也可以是Windows系统上的`C:`）
* 可选的根目录是一个路径分隔符
* 相对路径是一系列路径分隔符分割的文件名

一个路径分隔符可以是一个或多个`'/'`或实现特定的路径分隔符。

可移植的路径的例子有：

```
//host1/bin/hello.txt
.
tmp/
/a/b//../c
```

注意最后一个路径指向路径`/a/c`，在POSIX系统上它是一个绝对路径，但在Windows系统上它是一个相对路径（因为缺少驱动器名/盘符）。

另一方面像`C:/bin`这样的路径在Windows上是一个绝对路径（"C"盘/驱动器下的根目录"bin"）但在POSIX系统上它是一个相对目录（目录"C:"里的"bin"子目录）。

在Windows系统上反斜杠是特定实现的路径分隔符，因此路径也可以使用反斜杠作为路径分隔符：

```
\\host1\bin\hello.txt
.
tmp\
\a\b\...\c
```

一个路径可能是空的。这意味着没有路径，和"."不同。

### 19.2.4 规范化

一个路径可以被规范化。在一个规范化路径中：

* 文件名只用单个路径分隔符分隔
* 除非整个路径名就是一个单独的"."否则不应该使用"."
* 文件名不包括".."除非它们位于一个相对路径的起始处
* 如果路径中是一个有名字的目录而不是"."或者".."的话那么该路径应该以一个路径分隔符作为结尾

注意一个以路径分隔符结尾的文件名不同于不以分隔符结尾的文件名。原因是在某些操作系统上当路径是一个目录的时候最后有没有分割符将导致不同的行为（例如，如果有一个分隔符符号链接可能会被解析）。

路径规范化作用表（表19.1）列出了一些POSIX和Windows系统上规范化的例子。注意在POSIX系统上`C:bar`和`C:`只是文件名，并没有像Windows上那样代表一个特定的分区。

### 许多细节正在完善

### 19.4 后记

文件系统库在Beman Dawes的领导下已经作为`Boost`库开发了很多年了。在2014年它第一次变成了正式的测试版标准--文件系统库技术规范（见[https://wg21.link/n4100](https://wg21.link/n4100)）。

在[https://wg21.link/p0218r0](https://wg21.link/p0218r0)上文件系统库技术规范被Beman Dawes提议加入标准库。对相对路径的计算的支持由Beman Dawes，Nicolai Josuttis和Jamie Allsop在[https://wg21.link/p0219r1](https://wg21.link/p0219r1)上添加。一些细小的错误修复由Beman Dawes在[https://wg21.link/p0317r1](https://wg21.link/p0317r1)上、Nicolai Josuttis在[https://wg21.link/p0392r0](https://wg21.link/p0392r0)上、Jason Liu和Hubert Tong在[https://wg21.link/p0430r2](https://wg21.link/p0430r2)上提出，尤其是文件系统小组的成员（Beman Dawes，S. Davis Herring，Nicolai Josuttis，Jason Liu，Billy O’Neal，P.J. Plauger和Jonathan Wakely）在[https://wg21.link/p0492r2](https://wg21.link/p0492r2)上提出的修复。


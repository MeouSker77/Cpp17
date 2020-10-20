#include <filesystem>
#include <iostream>

void testSymLink(std::filesystem::path top)
{
    top = absolute(top);    // 转换为绝对路径，方便切换当前目录时
    create_directory(top);  // 确保top存在
    current_path(top);      // 然后切换当前目录
    std::cout << std::filesystem::current_path() << '\n'; // 打印top的路径

    // 定义我们自己的子目录（还没有创建）：
    std::filesystem::path px{top / "a/x"};
    std::filesystem::path py{top / "a/y"};
    std::filesystem::path ps{top / "a/s"};

    // 打印出一些相对路径（这些文件还不存在）：
    std::cout << px.relative_path() << '\n';        // px的相对路径
    std::cout << px.lexically_relative(py) << '\n'; // 从py到px的路径："../x"
    std::cout << relative(px, py) << '\n';          // 从py到px的路径："../x"
    std::cout << relative(px) << '\n';              // 从当前路径到px的路径："a/x"

    std::cout << px.lexically_relative(ps) << '\n'; // 从ps到px的路径："../x"
    std::cout << relative(px, ps) << '\n';          // 从ps到px的路径："../x"

    // 现在创建子目录和符号链接
    create_directories(px);                         // 创建"top/a/x"
    create_directories(py);                         // 创建"top/a/y"
    if (!is_symlink(ps)) {
        create_directory_symlink(top, ps)           // 创建"top/a/s" -> "top"
    }
    std::cout << "ps: " << ps << '\n' << " -> " << read_symlink(ps) << '\n';

    // 观察词法处理和文件系统处理的相对路径的不同：
    std::cout << px.lexically_relative(ps) << '\n'; // ps到px的路径："../x"
    std::cout << relative(px, ps) << '\n';          // ps到px的路径："a/x"
}
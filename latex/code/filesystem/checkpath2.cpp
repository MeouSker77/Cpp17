#include <iostream>
#include <filesystem>
#include <cstdlib>  // for EXIT_FAILURE

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <path> \n";
        return EXIT_FAILURE;
    }
    std::filesystem::path p{argv[1]};   // p代表一个文件系统路径（可能不存在）
    if (is_regular_file(p)) {   // 路径p是普通文件吗？
        std::cout << '"' << p.string() << "\" existes with " << file_size(p) << " bytes\n";
    }
    else if (is_directory(p)) { // 路径p是目录吗？
        std::cout << '"' << p.string() << "\" is a directory containing:\n";
        for (const auto& e : std::filesystem::directory_iterator{p}) {
            std::cout << "  \"" << e.path().string() << "\"\n";
        }
    }
    else if (exists(p)) {       // 路径p存在吗？
        std::cout << '"' << p.string() << "\" is a special file\n";
    }
    else {
        std::cout << "path \"" << p.string() << "\" does not exist\n";
    }
}
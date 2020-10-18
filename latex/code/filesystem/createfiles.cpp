#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib> // for std::exit()和EXIT_FAILURE

int main()
{
    namespace fs = std::filesystem;
    try {
        // 创建目录tmp/test/（如果不存在的话）
        fs::path testDir{"tmp/test"};
        create_directories(testDir);

        // 创建数据文件tmp/test/data.txt：
        auto testFile = testDir / "data.txt";
        std::ofstream  dataFile{testFile};
        if (!dataFile) {
            std::cerr << "OOPS, can't open\""
                      << testFile.string() << "\"\n";
            std::exit(EXIT_FAILURE);    // 以失败方式结束程序
        }
        dataFile << "The answer is 42\n";

        // 创建符号链接tmp/slink/，指向tmp/test/：
        create_directory_symlink("test",
                                 testDir.parent_path() / "slink");
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "EXCEPTION: " << e.what() << '\n';
        std::cerr << "    path1: \"" << e.path1().string() << "\"\n";
    }

    // 递归列出所有文件（同时遍历符号链接）
    std::cout << fs::current_path().string() << ":\n";
    auto iterOpts{fs::directory_options::follow_directory_symlink};
    for (const auto& e : fs::recursive_directory_iterator(".", iterOpts)) {
        std::cout << "  " << e.path().lexically_normal().string() << '\n';
    }
}
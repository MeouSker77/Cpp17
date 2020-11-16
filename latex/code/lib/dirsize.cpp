#include <vector>
#include <iostream>
#include <numeric>      // for transform_reduce()
#include <execution>    // for 执行策略
#include <filesystem>   // 文件系统库
#include <cstdlib>      // for EXIT_FAILURE

int main(int argc, char *argv[]) {
    // 根目录作为命令行参数传递：
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <path> \n";
        return EXIT_FAILURE;
    }
    std::filesystem::path root{argv[1]};

    // 初始化文件树中所有文件路径的列表：
    std::vector<std::filesystem::path> paths;
    try {
        std::filesystem::recursive_directory_iterator dirpos{root};
        std::copy(begin(dirpos), end(dirpos), std::back_inserter(paths));
    }
    catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // 累积所有普通文件的大小：
    auto sz = std::transform_reduce(
                    std::execution::par,                    // 并行执行
                    paths.cbegin(), paths.cend(),           // 范围
                    std::uintmax_t{0},                      // 初始值
                    std::plus<>(),                          // 累加...
                    [] (const std::filesystem::path& p) {    // 如果是普通文件返回文件大小
                        return is_regular_file(p) ? file_size(p) : std::uintmax_t{0};
                    });
    std::cout << "size of all " << paths.size()
              << " regular files: " << sz << '\n';
}
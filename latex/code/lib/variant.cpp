#include <variant>
#include <iostream>

int main()
{
    std::variant<int, std::string> var{"hi"};   // 初始化为string选项
    std::cout << var.index() << '\n';           // 打印出1
    var = 42;                                   // 现在持有int选项
    std::cout << var.index() << '\n';           // 打印出0
    ...
    try {
        int i = std::get<0>(var);               // 通过索引访问
        std::string s = std::get<std::string>(var); // 通过类型访问（这里会抛出异常）
        ...
    }
    catch (const std::bad_variant_access& e) {  // 当索引/类型错误时进行处理
        std::cerr << "EXCEPTION: " << e.what() << '\n';
        ...
    }
}
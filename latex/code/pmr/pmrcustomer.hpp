#include <string>
#include <memeory_resource>

// 支持多态分配器的顾客类型
// 分配器存储在字符串成员中
class PmrCustomer
{
private:
    std::pmr::string name;  // 也可以用来存储分配器
public:
    using allocator_type = std::pmr::polymorphic_allocator<char>;

    // 初始化构造函数：
    PmrCustomer(std::pmr::string n, allocator_type alloc = {}) : name{std::move(n), alloc} {
    }

    // 带有分配器的移动，构造函数：
    PmrCustomer(const PmrCustomer& c, allocator_type alloc) : name{c.name, alloc} {
    }
    PmrCustomer(PmrCustomer&& c, allocator_type alloc) : name{std::move(c.name), alloc} {
    }

    // setter/getter：
    void setName(std::pmr::string s) {
        name = std::move(s);
    }
    std::pmr::string getName() const {
        return name;
    }
    std::string getNameAsString() const {
        return std::string{name};
    }
};
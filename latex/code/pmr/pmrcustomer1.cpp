#include "pmrcustomer.hpp"
#include "tracker.hpp"
#include <vector>

int main()
{
    Tracker tracker;
    std::pmr::vector<PmrCustomer> coll(&tracker);
    coll.reserve(100);                      // 使用tracker分配

    PmrCustomer c1{"Peter, Paul & Mary"};   // 使用get_default_resource()分配
    coll.push_back(c1);                     // 使用vector的分配器(tracker)分配
    coll.push_back(std::move(c1));          // 拷贝（分配器不可交换）

    for (const auto& cust : coll) {
        std::cout << cust.getName() << '\n';
    }
}
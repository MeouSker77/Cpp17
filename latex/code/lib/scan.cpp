#include <numeric>
#include <iostream>
#include <iterator>
#include <array>

int main()
{
    std::array coll{3, 1, 7, 0, 4, 1, 6, 3};

    std::cout << " inclusive_scan():   ";
    std::inclusive_scan(coll.begin(), coll.end(),
                        std::ostream_iterator<int>(std::cout, " "));

    std::cout << "\n exclusive_scan(): ";
    std::exclusive_scan(coll.begin(), coll.end(),
                        std::ostream_iterator<int>(std::cout, " "),
                        0);     // 必须的

    std::cout << "\n exclusive_scan(): ";
    std::exclusive_scan(coll.begin(), coll.end(),
                        std::ostream_iterator<int>(std::cout, " "),
                        100);   // 必须的

    std::cout << "\n inclusive_scan():     ";
    std::inclusive_scan(coll.begin(), coll.end(),
                        std::ostream_iterator<int>(std::cout, " "),
                        std::plus{}, 100);

    std::cout << "\n exclusive_scan(): ";
    std::exclusive_scan(coll.begin(), coll.end(),
                        std::ostream_iterator<int>(std::cout, " "),
                        100, std::plus{});  // 注意：参数顺序不同
}
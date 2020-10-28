#include "incomplete.hpp"
#include <iostream>

int main()
{
    // 创建节点树：
    Node root{"top"};
    root.add(Node{"elem1"});
    root.add(Node{"elem2"});
    root[0].add(Node{"elem1.1"});

    // 打印节点树：
    root.print();
}
#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <iostream>
#include <string>

class Node
{
  private:
    std::string value;
    std::vector<Node> children; // 自从C++17起OK（这里Node是一个不完全类型）
  public:
    // 用单个值创建一个Node：
    Node(std::string s) : value{std::move(s)}, childern{} {
    }

    // 添加子节点：
    void add(Node n) {
        children.push_back(std::move(n));
    }

    // 访问子节点：
    Node& operator[](std::size_t idx) {
        return children.at(idx);
    }

    // 递归打印出节点树：
    void print(int indent = 0) const {
        std::cout << std::string(indent, ' ') << value << '\n';
        for (const auto& n : children) {
            n.print(indent + 2);
        }
    }
    ...
};

#endif  // NODE_HPP
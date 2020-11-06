// 定义二叉树结构和遍历辅助函数
struct Node {
    int value;
    Node *subLeft{nullptr};
    Node *subRight{nullptr};
    Node(int i = 0) : value{i} {
    }
    int getValue() const {
        return value;
    }
    ...
    // 遍历辅助函数：
    static constexpr auto left = &Node::subLeft;
    static constexpr auto right = &Node::subRight;

    // 使用折叠表达式遍历树：
    template<typename T, typename... TP>
    static Node* traverse(T np, TP... paths) {
        return (np ->* ... ->* paths);  // np ->* paths1 ->* paths2
    }
};
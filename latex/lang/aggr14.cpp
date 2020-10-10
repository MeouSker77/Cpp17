struct Derived;

struct Base {
    friend struct Derived;
private:
    Base() {
    }
};

struct Derived : Base {
};

int main()
{
    Derived d1{};   // 自从C++17起ERROR
    Derived d2;     // 仍然OK（但可能不会初始化）
}
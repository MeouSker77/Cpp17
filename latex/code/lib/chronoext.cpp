#include <chrono>
#include <iostream>

std::ostream &operator<<(std::ostream &strm,
                         const std::chrono::duration<double, std::milli>& dur)
{
    return strm << dur.count() << "ms";
}

template<typename T>
void roundAndAbs(T dur)
{
    using namespace std::chrono;

    std::cout << dur << '\n';
    std::cout << " abs():   " << abs(dur) << '\n';
    std::cout << " cast:    " << duration_cast<std::chrono::seconds>(dur) << '\n';
    std::cout << " floor(): " << floor<std::chrono::seconds>(dur) << '\n';
    std::cout << " ceil():  " << ceil<std::chrono::seconds>(dur) << '\n';
    std::cout << " round(): " << round<std::chrono::seconds>(dur) << '\n';
}

int main()
{
    using namespace std::literals;
    roundAndAbs(3.33s);
    roundAndAbs(3.77s);
    roundAndAbs(-3.77s);
}

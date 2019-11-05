#include <iostream>
using namespace std;

template <typename T>
struct larger_than
{
    T dummy[2];
};

using no_type = char;
using yes_type = larger_than<no_type>;

template <typename L, typename R>
class has_conversion
{
    static yes_type test(R);
    static no_type test(...);
    static L left();    

public:

    static const bool L2R = sizeof(test(left())) == sizeof(yes_type);
    static const bool identity = false;
};

template <typename T>
class has_conversion<T, T>
{
public:
    static const bool L2R = true;
    static const bool identity = true;
};

int main()
{
    cout << boolalpha << has_conversion<int&, int>::L2R << '\n';
    cout << boolalpha << has_conversion<int, int&>::L2R << '\n';
    
    cout << boolalpha << has_conversion<int const&, int>::L2R << '\n';
    cout << boolalpha << has_conversion<int, int const&>::L2R << '\n';
    
    return 0;   
}


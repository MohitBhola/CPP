#include <iostream>
using namespace std;

template <typename T, typename = void_t<>>
struct IsClass : public false_type
{};

template <typename T>
struct IsClass<T, void_t<int T::*>> : public true_type
{};

/*
template <typename T>
class IsClass
{
    template <typename U>
    static yes_type test(int U::*);
    
    template <typename U>
    static no_type test(...);
    
public:

    static const int value = sizeof(test<T>(0)) != sizeof(no_type);
};
*/

struct Foo
{};

int main()
{
    cout << boolalpha << IsClass<Foo>::value << '\n';
    cout << boolalpha << IsClass<int>::value << '\n';
    return 0;
}

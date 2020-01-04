#include <iostream>
#include <utility>
#include <type_traits>

using namespace std;

struct Foo
{
    Foo() = delete;
};

struct Bar
{
    Bar() = default;
};

template <typename T>
class IsDefaultConstructibleHelper
{
    template <typename X, typename = decltype(X())>
    static true_type test(X*);
    
    template <typename X>
    static false_type test(...);
    
public:

    using type = decltype(test<T>(nullptr));
};

template <typename T>
struct IsDefaultConstructible : IsDefaultConstructibleHelper<T>::type
{};

// alternate solution based on SFINAE'ing out partial specialization
/*
template <typename T, typename = std::void_t<>>
struct IsDefaultConstructible : std::false_type
{};

template <typename T>
struct IsDefaultConstructible<T, std::void_t<decltype(T())>> : std::true_type
{};
*/

int main()
{
    std::cout << IsDefaultConstructible<Foo>::value << '\n';
    std::cout << IsDefaultConstructible<Bar>::value << '\n';
    
    return 0;
}

/*
0
1
*/

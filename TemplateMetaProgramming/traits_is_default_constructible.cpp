#include <iostream>
#include <utility>
#include <type_traits>

struct Foo
{
    Foo() = delete;
};

struct Bar
{
    Bar() = default;
};

template <typename T, typename = std::void_t<>>
struct IsDefaultConstructible : std::false_type
{};

template <typename T>
struct IsDefaultConstructible<T, std::void_t<decltype(T())>> : std::true_type
{};

int main()
{
    
    std::cout << IsDefaultConstructible<Foo>::value << '\n';
    std::cout << IsDefaultConstructible<Bar>::value << '\n';
    
    return 0;
}

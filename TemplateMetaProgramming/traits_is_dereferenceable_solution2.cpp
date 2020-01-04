#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>

template <typename T, typename = std::void_t<>>
struct is_dereferenceable : std::false_type
{};

template <typename T>
struct is_dereferenceable<T, std::void_t<decltype(*std::declval<T>())>> : std::true_type
{};

int main()
{
    std::cout << is_dereferenceable<int>::value << '\n';
    std::cout << is_dereferenceable<std::shared_ptr<int>>::value << '\n';
    
    return 0;
}

/*
0
1
*/

#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>

using no_type = char;

template <std::size_t N>
struct yes_type
{
    char dummy[2];
};

template <typename T>
class is_dereferenceable_helper
{
    template <typename U>
    static yes_type<sizeof(*std::declval<U>())> test(U*);
    
    template <typename>
    static no_type test(...);
    
    template <std::size_t N>
    static std::true_type cast(yes_type<N>);
    
    static std::false_type cast(no_type);
    
public:

    using type = decltype(cast(test<T>(nullptr)));
};

template <typename T>
struct is_dereferenceable : is_dereferenceable_helper<T>::type
{};

int main()
{
    std::cout << is_dereferenceable<int>::value << '\n';
    std::cout << is_dereferenceable<std::shared_ptr<int>>::value << '\n';
    
    return 0;
}

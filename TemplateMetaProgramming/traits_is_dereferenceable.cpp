#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>

template <typename T>
class is_dereferenceable_helper
{
    template <typename U, typename = decltype(*std::declval<U>())>
    static std::true_type test(U*);
    
    template <typename>
    static std::false_type test(...);
    
public:

    using type = decltype(test<T>(nullptr));
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

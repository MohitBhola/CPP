#include <iostream>
#include <utility>
#include <type_traits>

using namespace std;

struct Base
{};

struct Derived : Base
{};

template <typename FROM, typename TO>
class IsConvertibleHelper
{
    template <typename X>
    static true_type test(X);
    
    template <typename X>
    static false_type test(...);
    
public: 

    using type = decltype(test<TO>(declval<FROM>()));
};

template <typename FROM, typename TO>
struct IsConvertible : IsConvertibleHelper<FROM, TO>::type
{};

// alternate solution based on SFINAE'ing out partial specialization
/*
template<
    typename FROM, 
    typename TO, 
    bool = std::is_array<TO>::value || std::is_function<TO>::value || std::is_void<TO>::value,
    typename = std::void_t<>>
struct IsConvertibleT : std::integral_constant<bool, std::is_void<FROM>::value && std::is_void<TO>::value>
{};

template <typename FROM, typename TO>
struct IsConvertibleT<FROM, TO, false, std::void_t<decltype(IsConvertibleHelperT<TO>::aux(std::declval<FROM>()))>> : std::true_type
{};
*/

int main()
{
    std::cout << IsConvertible<int, unsigned int>::value << '\n';
    std::cout << IsConvertible<int, char*>::value << '\n';
    
    std::cout << IsConvertible<Derived, Base>::value << '\n';
    std::cout << IsConvertible<Base, Derived>::value << '\n';
    
    return 0;
}

/*
1
0
1
0
*/

#include <iostream>
#include <utility>
#include <type_traits>

struct Base
{};

struct Derived : Base
{};

template <typename T>
struct IsConvertibleHelperT
{
	static void aux(T);
};

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

int main()
{
    std::cout << IsConvertibleT<int, unsigned int>::value << '\n';
    std::cout << IsConvertibleT<int, char*>::value << '\n';
    
    std::cout << IsConvertibleT<Derived, Base>::value << '\n';
    std::cout << IsConvertibleT<Base, Derived>::value << '\n';
    
    return 0;
}

// sometimes, the simplest form of assertion just *tries to use* what we require
#include <cstdlib>

template <std::size_t N>
struct Foo
{
    constexpr static std::size_t value = N;
    using type = char[N];
}; 

template <typename T>
void suppressUnusedWarning(T const& t)
{}

template <typename T>
void Bar()
{
    // OK only if T has a publicly available member type type
    using ERROR_T_DOES_NOT_CONTAIN_type = typename T::type;
    suppressUnusedWarning(ERROR_T_DOES_NOT_CONTAIN_type{});
    
    // OK only if T has a publicly available member constant value
    decltype(T::value) ERROR_T_DOES_NOT_CONTAIN_value(T::value);
    suppressUnusedWarning(ERROR_T_DOES_NOT_CONTAIN_value);
}    

int main()
{    
    Bar<Foo<42>>(); // OK    
    return 0;
}

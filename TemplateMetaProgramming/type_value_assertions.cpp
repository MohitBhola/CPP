// sometimes, the simplest form of assertion just *tries to use* what we require

template<
    std::size_t N>
struct Foo
{
    constexpr static std::size_t value = N;
    using type = char[N];
};    

template<
    typename T>
void Bar()
{
    // OK only *if* T has a publicly available member type type
    using ERROR_T_DOES_NOT_CONTAIN_type = typename T::type;
    
    // OK only *if* T has a publicly available member contant value
    decltype(T::value) ERROR_T_DOES_NOT_CONTAIN_value(T::value);
}    

int main()
{    
    Bar<Foo<42>>(); // OK
    
    return 0;
}

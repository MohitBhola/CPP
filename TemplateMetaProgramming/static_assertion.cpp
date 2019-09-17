// primary template
template<
    bool condition>
struct static_assertion;

template<>
struct static_assertion<true>
{
    static_assertion() = default;
    
    template<
        typename T>
    static_assertion(T) {}
};

int main()
{    
    const static_assertion<(char(255)>0)> ASSERT2("char is unsigned");
      
    return 0;
}

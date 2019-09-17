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

#define ASSERT(condition) sizeof(static_assertion<condition>)

int main()
{    
    const static_assertion<(char(255)>0)> ASSERT2("char is unsigned");
    ASSERT((char(255) > 0));
      
    return 0;
}

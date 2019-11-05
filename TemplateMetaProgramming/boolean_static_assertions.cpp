// primary template
// intentionally undefined
template <bool condition>
struct static_assertion;

// defined only for true conditions
// so, attempt to instantiate the template static_assertion with a false condition won't compile
template <>
struct static_assertion<true>
{
    static_assertion() = default;
    
    template <typename T>
    static_assertion(T) {}
};

#define ASSERT(condition) sizeof(static_assertion<condition>)

int main()
{    
    const static_assertion<(char(255)>0)> ASSERT2("char is unsigned");
    ASSERT((char(255) > 0));
      
    return 0;
}

/*
main.cpp: In function 'int main()':
main.cpp:21:50: error: variable 'const static_assertion<false> ASSERT2' has initializer but incomplete type
   21 |     const static_assertion<(char(255)>0)> ASSERT2("char is unsigned");
      |                                                  ^
main.cpp:17:61: error: invalid application of 'sizeof' to incomplete type 'static_assertion<false>'
   17 | #define ASSERT(condition) sizeof(static_assertion<condition>)
      |                                                             ^
main.cpp:22:5: note: in expansion of macro 'ASSERT'
   22 |     ASSERT((char(255) > 0));
*/

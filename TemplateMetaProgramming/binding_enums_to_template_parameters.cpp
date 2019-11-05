/*
 * Binding an enum (named or unnamed) to a template parameter is futile
 * _if_ we intend to perform integer arithmetic on template function parameter(s)
 * or template function local variable(s)
 */
 
template <typename T>
struct Foo
{
    enum class Values {value = 42}; 
};    

template <typename T>
void f(T t)
{
    ++t;          // ERROR if T = named/unnamed enum
    (void)(t+42); // ERROR if T = named/unnamed enum
}    

int main(int argc, char **argv)
{
    f(Foo<int>::Values::value);                     // leads to ERRORs
    //f(static_cast<int>(Foo<int>::Values::value)); // an explicit cast to an integral type is the right thing to do here
    
    return 0;
}


/*
 * Binding an enum (named or unnamed) to a template parameter is futile
 * *if* we want to perform integer arithmetic on template function parameter(s)
 * or template function local variable(s)
 */
 
template<
    typename T>
struct Foo
{
    // the compiler would never reserve storage for an enum
    // it would be a compile time error for client code to take its address
    enum class Values {value = 42}; 
};    

template<
    typename T>
void f(T t)
{
    ++t; // ERROR
    (void)(t+42); // ERROR
}    

int main(int argc, char **argv)
{
    f(Foo<int>::Values::value);
    
    return 0;
}


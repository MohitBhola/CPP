#include <iostream>
using namespace std;

template<
    typename T>
struct Foo
{
    // the compiler would never reserve storage for an enum
    // it would be a compile time error for client code to take its address
    enum {value = 42}; 
};    

template<
    typename T>
struct Bar
{
    // the compiler would always reserve storage for a static data member
    // client code can take its address
    inline static int const value = 42;
};    

/*
template<
    typename T>
int const Bar<T>::value;    
 */ 

int main(int argc, char **argv)
{
    // ERROR
    //cout << &Foo<int>::value << '\n';
    
    // fine
    cout << &Bar<int>::value << '\n';
    
    return 0;
}

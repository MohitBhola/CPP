#include <string>
#include <vector>
#include <iostream>
using namespace std;

template <typename T>
struct Foo
{
    vector<T> values;

    // ctor # 1
    Foo(T const& t) : values({t}) 
    {
        cout << "Foo(T const& t)" << '\n';
    }
    
    // ctor # 2
    Foo(Foo const& other) : values(other.values) 
    {
        cout << "Foo(Foo const& other)" << '\n';;
    }
    
    // ctor # 3
    Foo& operator=(Foo const& other) 
    {
        values = other.values;
        cout << "Foo(Foo const& other)" << '\n';
    }
};

// if a string literal is passed to the Foo ctor (ctor # 1), deduce a Foo<string> 
// and implicitly convert that string literal to the string parameter expected by the Foo ctor
Foo(char const*) -> Foo<string>;

int main()
{
    // thnks to CTAD, Foo<string> is deduced
    // it is an ERROR to try to copy initialize an object (that is, via operator = )
    // by passing a string literal to a ctor expecting a std::string
    //Foo foo1 = "xyz"; 
    
    // this works fine!
    Foo foo1("xyz"); 
    
    // the following declarations declare the same, that is, Foo<string>
    // thus, copy construction gets invoked instead of creating a Foo<string> with an element that itself is a Foo<string>
    Foo foo2(foo1);
    Foo foo3(foo1);
    Foo foo4 = {foo1};
    
    return 0;
}

/*
Foo(T const& t)
Foo(Foo const& other)
Foo(Foo const& other)
Foo(Foo const& other)
*/

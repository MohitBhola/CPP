#include <iostream>
#include <string>
using namespace std;

// a simplified pair
template<
    typename T1,
    typename T2>
struct Foo
{
    T1 first{};
    T2 second{};
    
    Foo()
    {
        cout << "Default ctor" << '\n';
    }
    
    Foo(T1 t1, T2 t2)
    : first(t1), second(t2)
    {
        cout << "Explicit ctor" << '\n';
    }
    
    Foo(Foo const&)
    {
        cout << "Normal copy ctor" << '\n';
    }
    
    template<
        typename S1,
        typename S2>
    Foo(Foo<S1, S2> const& other)
    : first(other.first), second(other.second)
    {
        cout << "Universal copy ctor" << '\n';
    }    
};

void Bar(Foo<int, string> const& foo)
{}

int main(int argc, char **argv)
{   
    Foo<int, string> foo1{1, "abc"};
    // foo1 (Foo<int, string>) is an exact match for the parameter type of: Bar(Foo<int, string> const&)
    Bar(foo1);
    
    Foo<int const, string> foo2{2, "xyz"}
    // foo2 (Foo<int const, string>) is a mismatch for the parameter type of: Bar(Foo<int, string> const&)
    // a temporary (of type Foo<int, string>) is thus created from foo2 (which is of type Foo<int const, string>)
    // thus the extraneous copy
    Bar(foo2);
    
    return 0;
}

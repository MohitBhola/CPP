#include <iostream>
#include <string>
using namespace std;

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

/*
template<
    typename X,
    typename Y>
void Bar(Foo<X, Y> const& foo1, Foo<X, Y> const& foo2)
{}
 */ 

int main(int argc, char **argv)
{
    Foo<int, string> foo1{1, "abc"};
    Foo<const int, string> foo2{2, "xyz"};
    
    Bar(foo1);
    Bar(foo2);
	
	return 0;
}

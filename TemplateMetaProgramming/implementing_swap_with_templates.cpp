#include <vector>
#include <iostream>
 
namespace ns_bar
{

// to optimize swap, provide a public member function swap that...
class Bar
{
    std::vector<int> ivec_{};
    
public:

    Bar(int n, int v) : ivec_(n, v) {}
   
    // 1. ...provides a using declaration for std::swap
    // 2. invokes *unqualified swap* on data members...
    void swap(Bar& other)
    {
        std::cout << "Bar::swap()" << '\n';
        
        using std::swap;
        swap(ivec_, other.ivec_);
    }
};

}

namespace std
{
    // ...and, most importantly, provide a full specialization of 
    // std::swap for the class type under consideration
    template<>
    void swap<ns_bar::Bar>(ns_bar::Bar& bar1, ns_bar::Bar& bar2)
    {
        bar1.swap(bar2);
    }
}

namespace ns_foo
{
    
template<
    typename T>
class Foo
{
    T t_{};
    
public:
    
    Foo(T t) : t_(t) {}
 
    void swap(Foo<T>& other)
    {
        using std::swap;
        swap(t_, other.t_);
    }
};

}    

int main(int argc, char **argv)
{
    ns_bar::Bar bar1{42, 42};
    ns_bar::Bar bar2{43, 43};
    
    ns_foo::Foo<ns_bar::Bar> iFoo1{bar1};
    ns_foo::Foo<ns_bar::Bar> iFoo2{bar2};
    
    iFoo1.swap(iFoo2);
    
    return 0;
}

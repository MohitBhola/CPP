#include <vector>
#include <iostream>
 
namespace ns_bar
{
    
class Bar
{
    std::vector<int> ivec_{};
    
public:

    Bar(int n, int v) : ivec_(n, v) {}
    
    void swap(Bar& other)
    {
        std::cout << "Bar::swap()" << '\n';
        using std::swap;
        swap(ivec_, other.ivec_);
    }
};

void swap(Bar& a, Bar& b)
{
    a.swap(b);
}

}
 
template<
    typename T>
void swap_with_ADL(T& a, T& b)
{
    // this would redirect to a swap(...) function defined in the same namespace as T, if there is any,
    // else to std::swap
    using std::swap;
    swap(a, b);
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

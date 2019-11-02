#include <iostream>
using namespace std;

constexpr int highest_index_helper(unsigned N)
{
    int index = -1;
    
    if (N == 0)
        return index;
        
    unsigned accumulator = 1;
    
    while (accumulator <= N)
    {
        accumulator *= 2;
        ++index;
    }
    
    return index;
}

enum Interfaces : int
{
    none = -1,
    A = 1,
    B = 2,
    C = 4
};

template <int INTERFACE = Interfaces::none>
struct interface_traits;

template <>
struct interface_traits<Interfaces::A>
{
    void funcA()
    {
        cout << "funcA()" << '\n';
    }
};

template <>
struct interface_traits<Interfaces::B>
{
    void funcB()
    {
        cout << "funcB()" << '\n';
    }
};

template <>
struct interface_traits<Interfaces::C>
{
    void funcC()
    {
        cout << "funcC()" << '\n';
    }
};

template <int InterfaceFlags>
struct interface_traits 
: interface_traits<InterfaceFlags & (1 << highest_index_helper(InterfaceFlags))>
, interface_traits<InterfaceFlags - (1 << highest_index_helper(InterfaceFlags))>
{};

/*
** Concrete class is a template taking an unsigned that encodes the supported functionality
*/
template <int InterfaceFlags>
struct ConcreteClass
: public interface_traits<InterfaceFlags>
{};

int main()
{    
    ConcreteClass<Interfaces::A | Interfaces::B> obj1;
    
    obj1.funcA();
    obj1.funcB();
    
    cout << "--------\n";
    
    ConcreteClass<Interfaces::C> obj2;
    
    obj2.funcC();
    
    return 0;
}

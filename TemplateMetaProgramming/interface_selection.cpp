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

// interface IDs
enum Interfaces : int
{
    none = -1,
    A = 1,
    B = 2,
    C = 4
};

template <int INTERFACE = Interfaces::none>
struct interface_traits;

// interface definitions
template <>
struct interface_traits<Interfaces::A>
{
    virtual void funcA() = 0;
};

template <>
struct interface_traits<Interfaces::B>
{
    virtual void funcB() = 0;
};

template <>
struct interface_traits<Interfaces::C>
{
    virtual void funcC() = 0;
};

// interface aggregation
template <int InterfaceFlags>
struct interface_traits 
: interface_traits<InterfaceFlags & (1 << highest_index_helper(InterfaceFlags))>
, interface_traits<InterfaceFlags - (1 << highest_index_helper(InterfaceFlags))>
{};

/*
** Every concrete class in the system is a template taking an unsigned that encodes the supported interfaces
*/
template <int InterfaceFlags>
struct ConcreteClass1
: public interface_traits<InterfaceFlags>
{
    void funcA()
    {
        cout << "MyInterface::funcA()" << '\n';
    }
    void funcB()
    {
        cout << "MyInterface::funcB()" << '\n';
    }
};

template <int InterfaceFlags>
struct ConcreteClass2
: public interface_traits<InterfaceFlags>
{
    void funcC()
    {
        cout << "MyInterface::funcC()" << '\n';
    }
};

int main()
{    
    ConcreteClass1<Interfaces::A | Interfaces::B> obj1;
    
    obj1.funcA();
    obj1.funcB();
    
    cout << "-------------------------------------\n";
    
    ConcreteClass2<Interfaces::C> obj2;
    
    obj2.funcC();
    
    return 0;
}

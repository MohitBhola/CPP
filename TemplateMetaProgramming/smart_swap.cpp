#include <vector>
#include <iostream>
#include <type_traits>
using namespace std;

template <typename T>
struct larger_than
{
    T dummy[2];
};

using no_type = char;
using yes_type = larger_than<no_type>;

struct swap_traits
{
    template <typename T>
    static void doSwap(T& a, T& b)
    {
        apply(a, b, test(&a));
    }
    
private:

    template <typename T, void (T::*F) (T&)>
    struct yes : yes_type
    {
        yes (int = 0) {}
    };
    
    template <typename T>
    static yes<T, &T::swap> test(T*)
    {
        return 0;
    }
    
    static no_type test(void*)
    {
        return 0;
    }
    
    template <typename T>
    static void apply(T& a, T& b, yes_type)
    {
        cout << "Doing an available member swap\n";
        a.swap(b);
    }
    
    template <typename T>
    static void apply(T& a, T& b, no_type)
    {
        cout << "Doing an ADL swap\n";
        using std::swap;
        swap(a,b);
    }
};

template <typename T>
void smart_swap(T&a, T& b)
{
    swap_traits::doSwap(a, b);
}

struct Foo
{
    int* pInt = nullptr;

    Foo() : pInt(new int(42)) {}
    Foo(int val) : pInt(new int(val)) {}
    
    void swap(Foo& that)
    {
        std::swap(pInt, that.pInt);
    }
    
    auto getThis() const
    {
        return this;
    }
    
    int getVal() const
    {
        return *pInt;
    }
    
    friend ostream& operator<<(ostream& os, Foo const& foo)
    {
        cout << "this: " << foo.getThis() << ", int val = " << foo.getVal(); 
        return os;
    }
};

namespace std {
    
    template <>
    void swap(Foo& x, Foo& y)
    {
        x.swap(y);
    }
}

int main()
{
    int a = 1;
    int b = 2;
    cout << "Before: a = " << a << ", b = " << b << '\n';
    smart_swap(a, b);
    cout << "After: a = " << a << ", b = " << b << '\n';
    
    Foo foo1{1};
    Foo foo2{2};
    cout << "Before: foo1 = " << foo1 << ", foo2 = " << foo2 << '\n';
    smart_swap(foo1, foo2);
    cout << "After: foo1 = " << foo1 << ", foo2 = " << foo2 << '\n';
        
    return 0;    
}

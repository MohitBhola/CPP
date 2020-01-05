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

    // for when T has void T::swap(T&)
    template <typename T, void (T::*F) (T&)>
    struct yes1 : yes_type
    {
        yes1 (int = 0) {}
    };
    
    // for when T has static void swap(T&, T&)
    template <typename T, void (*F) (T&, T&)>
    struct yes2 : yes_type
    {
        yes2 (int = 0) {}
    };
    
    template <typename T>
    static yes1<T, &T::swap>* test(T*)
    {
        return 0;
    }
    
    template <typename T>
    static yes2<T, &T::swap>* test(T*)
    {
        return 0;
    }
    
    static no_type test(void*)
    {
        return 0;
    }
    
    // for when T has void T::swap(T&)
    template <typename T>
    static void applyFurther(T& a, T& b, void (T::*F) (T&))
    {
        cout << "T has void T::swap(T&)" << '\n';
        a.swap(b);
    }
    
    // for when T has static void swap(T&, T&)
    template <typename T>
    static void applyFurther(T& a, T& b, void (*F)(T&, T&))
    {
        cout << "T has static void swap(T&, T&)" << '\n';
        T::swap(a, b);
    }
    
    // if any allowed T::swap exists, we land here
    template <typename T>
    static void apply(T& a, T& b, yes_type*)
    {
        cout << "Some T::swap exists.\n";
        applyFurther(a, b, &T::swap);
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

struct Bar
{
    int* pInt = nullptr;

    Bar() : pInt(new int(42)) {}
    Bar(int val) : pInt(new int(val)) {}
    
    static void swap(Bar& a, Bar& b)
    {
        std::swap(a.pInt, b.pInt);
    }
    
    auto getThis() const
    {
        return this;
    }
    
    int getVal() const
    {
        return *pInt;
    }
    
    friend ostream& operator<<(ostream& os, Bar const& bar)
    {
        cout << "this: " << bar.getThis() << ", int val = " << bar.getVal(); 
        return os;
    }
};

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
                
    Bar bar1{1};
    Bar bar2{2};
    cout << "Before: bar1 = " << bar1 << ", bar2 = " << bar2 << '\n';
    smart_swap(bar1, bar1);
    cout << "After: bar1 = " << bar1 << ", bar2 = " << bar2 << '\n';

    return 0;    
}

/*
Before: a = 1, b = 2
Doing an ADL swap
After: a = 2, b = 1
Before: foo1 = this: 0x7ffef1105fc0, int val = 1, foo2 = this: 0x7ffef1105fc8, int val = 2
Some T::swap exists.
T has void T::swap(T&)
After: foo1 = this: 0x7ffef1105fc0, int val = 2, foo2 = this: 0x7ffef1105fc8, int val = 1
Before: bar1 = this: 0x7ffef1105fd0, int val = 1, bar2 = this: 0x7ffef1105fd8, int val = 2
Some T::swap exists.
T has static void swap(T&, T&)
After: bar1 = this: 0x7ffef1105fd0, int val = 1, bar2 = this: 0x7ffef1105fd8, int val = 2
*/

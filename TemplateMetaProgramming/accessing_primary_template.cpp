#include <iostream>
using namespace std;

template <bool = false>
struct Foo
{
    static void func1()
    {
        cout << "Foo<false>::func1" << '\n';
    }
    
    static void func2()
    {
        cout << "Foo<false>::func2" << '\n';
    }
};

template <>
struct Foo<true>
{
    static void func1()
    {
        cout << "Foo<true>::func1" << '\n';
        
        // any specialization can access the corresponding function in the primary template
        Foo<false>::func1();
    }
    
    static void func2()
    {
        cout << "Foo<true>::func2" << '\n';
    }
};

int main()
{
    Foo<false>::func1();
    Foo<false>::func2();
    
    Foo<true>::func1();
    Foo<true>::func2();

    return 0;
}

/*
Foo<false>::func1
Foo<false>::func2
Foo<true>::func1
Foo<false>::func1
Foo<true>::func2
*/

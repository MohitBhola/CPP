#include <iostream>
using namespace std;

struct boolean_type_t
{
    bool true_;
};

using boolean_type = bool boolean_type_t::*;

struct Foo
{
    int val{};
    
    Foo(int val_) : val(val_) {}
    
    operator boolean_type() const
    {
        return val ? &boolean_type_t::true_ : 0;
    }
};

int main()
{
    Foo foo{42};
    
    if (foo)
    {
        cout << "YaY" << '\n';
    }
    
    return 0;
}

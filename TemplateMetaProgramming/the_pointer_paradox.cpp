#include <iostream>
using namespace std;

// for int* to bind to `T const*` in the ptr version of foo, const-ness needs to be added
// however, int* binds seamlessly to `T const&` in the ref version of foo by setting T=int*

// this example if known as *the pointer paradox* :)

template <typename T>
void foo(T const& t)
{
    cout << "ref" << '\n';
}

template <typename T>
void foo(T const* t)
{
    cout << "ptr" << '\n';
}

int main()
{
    int ival = 42;
    foo(&ival);
    
    return 0;
}

/*
ref
*/

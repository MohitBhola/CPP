#include <iostream>
using namespace std;

template <int N, int FAKE = 0>
struct A
{
    auto foo()
    {
        return N;
    }
};

// the hidden template parameter, FAKE, is used to transform
// different levels of inheritance into different physical types!
template <int N, int FAKE = 0>
struct B : A<N % 2, FAKE>, B<N / 2, FAKE + 1>
{
    auto foo()
    {
        return A<N % 2, FAKE>::foo();
    }
};

template <int FAKE>
struct B<0, FAKE>
{};

int main()
{
    cout << B<9>().foo() << '\n';
    
    return 0;
}

/*
1
*/

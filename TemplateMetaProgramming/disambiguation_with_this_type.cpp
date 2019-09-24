#include <iostream>
using namespace std;

template<
    int N,
    typename T>
struct A
{
    auto foo()
    {
        return N;
    }
};

template<
    int N>
struct B : A<N % 2, B<N>>, B<N / 2>
{
    auto foo()
    {
        return A<N % 2, B<N>>::foo();
    }
};

template<>
struct B<0>
{};

int main()
{
    cout << B<9>::foo << '\n';
    
    return 0;
}


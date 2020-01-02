#include <iostream>
using namespace std;

template <size_t N, bool isTinyNumber = (N<2)>
struct Fibonacci
{
    static const size_t value = Fibonacci<N-1>::value + Fibonacci<N-2>::value;
};

template <size_t N>
struct Fibonacci<N, true>
{
    static const size_t value = N;
};

int main()
{
    cout << Fibonacci<17>::value << '\n';    

    return 0;
}

/*
1597
*/

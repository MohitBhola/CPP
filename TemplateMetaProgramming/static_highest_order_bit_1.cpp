#include <iostream>
using namespace std;

template <size_t X, int N>
struct static_highest_bit_helper
{
    static const int value = ((X >> N) % 2) ? N : static_highest_bit_helper<X, N-1>::value;
};

template <size_t X>
struct static_highest_bit_helper<X, 0>
{
    static const int value = (X % 2) ? 0 : -1;
};

template <size_t X>
struct static_highest_bit : static_highest_bit_helper<X, 8*sizeof(size_t) - 1>
{};

int main()
{
    cout << static_highest_bit<31>::value << '\n';    

    return 0;
}

/*
4
*/

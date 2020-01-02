#include <iostream>
using namespace std;

template <size_t N, size_t I>
struct static_ith_digit_1
{
    static const size_t value = static_ith_digit_1<N/10, I-1>::value;
};

template <size_t N>
struct static_ith_digit_1<N, 0>
{
    static const size_t value = N % 10;
};

int main()
{
    cout << static_ith_digit_1<123456789,3>::value << '\n';    

    return 0;
}

/*
6
*/


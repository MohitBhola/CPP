#include <iostream>
using namespace std;

template <size_t N>
struct power_of_10
{
    static const size_t value = 10 * power_of_10<N-1>::value;
};

template <>
struct power_of_10<0>
{
    static const size_t value = 1;
};

template <size_t I, size_t N>
struct static_ith_digit_2
{
    static const size_t value = (N / power_of_10<I>::value) % 10;
};

int main()
{
    cout << static_ith_digit_2<123456789,3>::value << '\n';    

    return 0;
}

/*
6
*/


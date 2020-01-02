#include <iostream>
using namespace std;

template <size_t X, int N>
struct static_highest_bit_helper
{
private:
    static const size_t Y = X >> (N/2);

public:
    static const int value = (Y > 0) ? (N/2 + static_highest_bit_helper<Y, N - N/2>::value) : static_highest_bit_helper<X, N/2>::value;
};

template <size_t X>
struct static_highest_bit_helper<X, 1>
{
    static const int value = (X % 2) ? 0 : -1;
};

template <size_t X>
struct static_highest_bit : static_highest_bit_helper<X, 8*sizeof(size_t)>
{};

int main()
{
    cout << static_highest_bit<15>::value << '\n';    

    return 0;
}

/*
4
*/

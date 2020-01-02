#include <iostream>
using namespace std;

template <size_t M, size_t N>
struct static_raise;

template <size_t M>
struct static_raise<M, 0>
{
    static const size_t value = 1;
};

// compilation complexity : logN
template <size_t M, size_t N>
struct static_raise
{
private:
    static const size_t tmp = static_raise<M, N/2>::value;

public:
    static const size_t value = tmp * tmp * ((N % 2 == 0) ? 1 : M);
};

int main()
{
    cout << static_raise<3,3>::value << '\n';    

    return 0;
}

/*
27
*/


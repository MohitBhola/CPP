#include <cstddef>
#include <iostream>
using namespace std;

// fixed_size<N> can itself have any size >= N
// but, sizeof(fixed_size<N>::type == N), because sizeof(char) == 1, always, on all platforms
template <std::size_t N>
struct fixed_size
{
    using type = char[N];
};    

int main()
{
    static_assert(sizeof(fixed_size<42>::type) == 42);
    cout << sizeof(fixed_size<42>::type) << '\n';
    
    return 0;
}

/*
42
*/

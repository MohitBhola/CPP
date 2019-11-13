#include <iostream>
using namespace std;

#define COMPUTE_HASH(x,c) ((x) << 1) ^ (c)

template 
<
    char c0 = 0, char c1 = 0, char c2 = 0, char c3 = 0,
    size_t HASH = 0
>
struct static_hash
: static_hash<c1, c2, c3, 0, COMPUTE_HASH(HASH, c1)>
{};

template <size_t HASH>
struct static_hash<0,0,0,0,HASH>
: integral_constant<size_t, HASH>
{};

int main()
{
    cout << static_hash<'A','B','C'>::value << '\n';
    cout << static_hash<'C','B','A'>::value << '\n';
    return 0;
}

/*
398
394
*/





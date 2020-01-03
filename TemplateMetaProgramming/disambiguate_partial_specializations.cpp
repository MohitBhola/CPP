#include <iostream>
using namespace std;

// primary template
template <typename T, int = 0>
struct is_integer
{
    static const bool value = false;
};

// specializations
// on *plane 0*
template <>
struct is_integer<short>
{
    static const bool value = true;
};

template <>
struct is_integer<int>
{
    static const bool value = true;
};

template <>
struct is_integer<long>
{
    static const bool value = true;
};

using lint = long;

/*
this specialization is a redefinition of *struct is_integer<long>*, and thus an error
template <>
struct is_integer<lint>
{
    static const bool value = true;
};
*/

// this speciaization is OK
template <>
struct is_integer<lint, 1>
{
    static const bool value = true;
};

int main()
{
    cout << is_integer<short>::value << '\n';
    cout << is_integer<int>::value << '\n';
    cout << is_integer<long>::value << '\n';
    
    cout << is_integer<lint, 1>::value << '\n';
    
    return 0;
}

/*
1
1
1
1
*/

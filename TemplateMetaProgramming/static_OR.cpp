#include <iostream>
using namespace std;

template <bool B>
struct selector
{
    static const bool value = true;
};

template <>
struct selector<false>
{
    static const bool value = false;
};


template <bool B, typename T>
struct static_OR_helper : selector<T::value>
{};

template <typename T>
struct static_OR_helper<true, T> : selector<true>
{};

template <typename T1, typename T2>
struct static_OR : static_OR_helper<T1::value, T2>
{};

struct Foo
{
    static const bool value = false;
};

struct Bar
{
    static const bool value = false;
};

int main()
{
    cout << static_OR<Foo, Bar>::value << '\n';    

    return 0;
}

/*
0
*/

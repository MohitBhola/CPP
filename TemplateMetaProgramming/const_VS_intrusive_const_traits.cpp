#include <iostream>
using namespace std;

template <bool B>
struct selector;

template <>
struct selector<true>
{
    static const bool value = true;
};

template <>
struct selector<false>
{
    static const bool value = false;
};

template <typename T>
struct IsConst : selector<false>
{};

template <typename T>
struct IsConst<T const> : selector<true>
{};

template <typename T>
struct is_intrusive_const : selector<false>
{};

template <typename T>
struct is_intrusive_const<T const> : selector<true>
{};

template <typename T>
struct is_intrusive_const<T&> : is_intrusive_const<T>
{};

int main()
{
    using IntRef = int&;
    using ConstIntRef = int const&;
   
    // a reference, const or otherwise, is never const
    cout << IsConst<IntRef>::value << '\n';
    cout << IsConst<ConstIntRef>::value << '\n';
    
    // a *custom* implementation, however, checks for intrusive constness
    // ie, whether the referenced to type is const or not
    cout << is_intrusive_const<IntRef>::value << '\n';
    cout << is_intrusive_const<ConstIntRef>::value << '\n';
    
    return 0;
}

/*
0
0
0
1
*/



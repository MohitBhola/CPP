#include <iostream>
using namespace std;

template <typename T1, typename T2>
struct transfer_qualifiers
{
    using type = T2;
};

// for reference
template <typename T1, typename T2>
struct transfer_qualifiers<T1&, T2>
{
    using aux_t = typename transfer_qualifiers<T1, T2>::type;
    using type = aux_t&;
};

// for const
template <typename T1, typename T2>
struct transfer_qualifiers<T1 const, T2>
{
    using aux_t = typename transfer_qualifiers<T1, T2>::type;
    using type = aux_t const;
};

// for volatile
template <typename T1, typename T2>
struct transfer_qualifiers<T1 volatile, T2>
{
    using aux_t = typename transfer_qualifiers<T1, T2>::type;
    using type = aux_t volatile;
};

// for bounded array
template <typename T1, typename T2, int N>
struct transfer_qualifiers<T1[N], T2>
{
    using aux_t = typename transfer_qualifiers<T1, T2>::type;
    using type = aux_t[N];
};

// for bounded array of const
template <typename T1, typename T2, int N>
struct transfer_qualifiers<T1 const[N], T2>
{
    using aux_t = typename transfer_qualifiers<T1 const, T2>::type;
    using type = aux_t[N];
};

// for bounded array of volatile
template <typename T1, typename T2, int N>
struct transfer_qualifiers<T1 volatile[N], T2>
{
    using aux_t = typename transfer_qualifiers<T1 volatile, T2>::type;
    using type = aux_t[N];
};

// for unbounded array
template <typename T1, typename T2>
struct transfer_qualifiers<T1[], T2>
{
    using aux_t = typename transfer_qualifiers<T1, T2>::type;
    using type = aux_t[];
};

// for unbounded array of const
template <typename T1, typename T2>
struct transfer_qualifiers<T1 const[], T2>
{
    using aux_t = typename transfer_qualifiers<T1 const, T2>::type;
    using type = aux_t[];
};

// for unbounded array of const
template <typename T1, typename T2>
struct transfer_qualifiers<T1 volatile[], T2>
{
    using aux_t = typename transfer_qualifiers<T1 volatile, T2>::type;
    using type = aux_t[];
};

// note: no definition
template <typename T>
struct TD;

int main()
{
    using T1 = int const[4];
    using T2 = double;
    
    TD<transfer_qualifiers<T1, T2>::type> td;
    
    return 0;
}

/*
main.cpp: In function 'int main()':
main.cpp:91:43: error: aggregate 'TD<const double [4]> td' has incomplete type and cannot be defined
   91 |     TD<transfer_qualifiers<T1, T2>::type> td;
*/   

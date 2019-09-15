#include <type_traits>
#include <utility>
using namespace std;

// alternatives to defining a max template

// a single type for call parameters and return value
template <
	typename T>
T max (const T a, const T b)
{
    return b < a ? a : b;
}

// different types for the call parameter
// one of the those types as the overall return type
template <
	typename T1, 
	typename T2>
T1 max (const T1 a, const T2 b)
{
    return b < a ? a : b;
}

// C++14
// let the compiler figure out the return type from the return statement
template <
	typename T1, 
	typename T2>
auto max (const T1 a, const T2 b)
{
    return b < a ? a : b;
}

// C++11
// let the compiler figure out the return type via trailing return type mechanism
// note that the compiler would use the rules of operator?: called for parameters a and b,
// to find out return type of max() at compile time.
// Thus using true as the condition of operator?: is enough 
template <
	typename T1, 
	typename T2>
auto max (const T1 a, const T2 b) -> typename std::decay<decltype(true ? a : b)>::type
{
    return b < a ? a : b;
}

// C++11
// Using std::common_type_t type traits
template <
	typename T1, 
	typename T2>
std::common_type_t<T1,T2> max (const T1 a, const T2 b)
{
    return b < a ? a : b;
}

// C++14
// Using default template argument 
template <
    typename T1, 
    typename T2, 
    typename RT = std::decay_t<decltype(true ? declval<T1>() : declval<T2>())>>
RT max (const T1 a, const T2 b)
{
    return b < a ? a : b;
}

/*
// C++14
// Using default template argument
template <
    typename T1, 
    typename T2,
    typename RT = std::common_type_t<T1,T2>
    >
RT
max (const T1 a, const T2 b)
{
    return b < a ? a : b;
}
 */ 

int main(int argc, char **argv)
{
    return 0;
}

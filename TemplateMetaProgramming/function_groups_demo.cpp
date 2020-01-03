#include <iostream>
#include <utility>
#include <cmath>
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

template <typename... Ts>
using VoidT = void;

template <typename T, typename = VoidT<>>
struct has_abs_method : selector<false>
{};

template <typename T>
struct has_abs_method<T, VoidT<decltype(declval<T>().abs())>> : selector<true>
{};

struct MyScalar
{
    int ival{};
    
    explicit MyScalar(int val) : ival(val) {}
    
    int abs() const
    {
        cout << "MyScalar::abs" << '\n';
        return ival < 0 ? -ival : ival;
    }
};

// the group (primary template)
// it contains a single type of action for many types
template <typename scalar_t>
struct maths
{
    static auto abs(scalar_t const& val, selector<false>)
    {
        cout << "maths<scalar_t>::abs(scalar_t const& val, selector<false>)" << '\n';
        return val < 0 ? -val : val;
    }
    
    static auto abs(scalar_t const& val, selector<true>)
    {
        cout << "maths<scalar_t>::abs(scalar_t const& val, selector<true>)" << '\n';
        return val.abs();
    }
};

// the group specialized
template <>
struct maths<double>
{
    template <bool UNUSED>
    static double abs(double const& val, selector<UNUSED>)
    {
        cout << "maths<double>::abs" << '\n';
        return std::fabs(val);
    }
};

// companion global function
// it serves to dispatch the call to the correct member of the group
template <typename scalar_t>
inline auto absolute_value(scalar_t const& val)
{
    return maths<scalar_t>::abs(val, selector<has_abs_method<scalar_t>::value>());
}

int main()
{
    // hits the primary template; computes the absolute value *normally*
    cout << absolute_value(-42) << '\n';
    
    // hits the primary template; computes the absolute value via the abs() API of aScalar
    MyScalar aScalar{42};
    cout << absolute_value(aScalar) << '\n';
    
    // hits the specialization; disregards the presence / absence of the abs() API 
    // computes the absolute value via std::fabs() API
    cout << absolute_value(-42.42) << '\n';
    
    return 0;
}

/*
maths<scalar_t>::abs(scalar_t const& val, selector<false>)
42
maths<scalar_t>::abs(scalar_t const& val, selector<true>)
MyScalar::abs
42
maths<double>::abs
42.42
*/

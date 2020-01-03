#include <iostream>
#include <cmath>
using namespace std;

template <typename T>
struct instance_of
{
    using type = T;
};

struct tag_floating
{
    tag_floating() = default;
    
    // implicit conversion of instances of floating point types to tag_floating
    tag_floating(instance_of<float>) {}
    tag_floating(instance_of<double>) {}
    tag_floating(instance_of<long double>) {}
};

struct tag_int
{
    tag_int() = default;
    
    // implicit conversion of instances of integer types to tag_int
    tag_int(instance_of<short>) {}
    tag_int(instance_of<int>) {}
    tag_int(instance_of<long>) {}
    tag_int(instance_of<unsigned short>) {}
    tag_int(instance_of<unsigned int>) {}
    tag_int(instance_of<unsigned long>) {}
};

// the group (primary template): layer-2
// provides for overload selection (a single kind of action for many types)
template <typename scalar_t>
struct maths
{
    static auto abs(scalar_t const& val, tag_int)
    {
        cout << "maths<scalar_t>::abs(scalar_t const& val, tag_int)" << '\n';
        return val < 0 ? -val : val;
    }
    
    static auto abs(scalar_t const& val, tag_floating)
    {
        cout << "maths<scalar_t>::abs(scalar_t const& val, tag_floating)" << '\n';
        return std::fabs(val);
    }
};

// companion global function: layer-1
// deduces scalar_t and dispatches the call to an appropriate member of the correct function group
template <typename scalar_t>
inline auto absolute_value(scalar_t const& val)
{
    return maths<scalar_t>::abs(val, instance_of<scalar_t>());
}

int main()
{
    cout << absolute_value(-42) << '\n';
    cout << absolute_value(-42.42) << '\n';
    
    return 0;
}

/*
maths<scalar_t>::abs(scalar_t const& val, tag_int)
42
maths<scalar_t>::abs(scalar_t const& val, tag_floating)
42.42
*/

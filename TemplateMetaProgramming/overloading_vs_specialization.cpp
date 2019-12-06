#include <iostream>
using namespace std;

// overloading and specialization may look similar, but they are different
// it is important to understand that a function template acts as a single entity, along with its specializations.
//
// that is, during overload resolution, the compiler would first compare the primary function 
// template along with other overloads
//
// *if* the compile selects the primary function template, only then it would look at any available specializations 
// that might be a better match!!!

// primary template
template <typename T>
T sq(T const& t)
{
    cout << "template sq" << '\n';
    return t*t;
}

// overloading
double sq(double const& d)
{
    cout << "non-template sq(double)" << '\n';
    return d*d;
}

// specialization
template <>
double sq(double const& d)
{
    cout << "specialization sq(double)" << '\n';
    return d*d;
}

int main()
{
    cout << sq(3.14) << '\n';
    return 0;
}

/*
non-template sq(double)                                                                                                                                                               
9.8596
*/

#include <iostream>
using namespace std;

// function template specialization is legal only at the namespace level
// but not every compiler enforces this

// the *right* approach is to call a namespace level function template from within a class

template <typename T>
T gsq(T const& t)
{
    cout << "global sq: primary template" << '\n';
    return t*t;
}

template <>
double gsq(double const& d)
{
    cout << "global sq: specialization for double" << '\n';
    return d*d;
}

struct Maths
{
    template <typename T>
    static auto sq(T const& t)
    {
        cout << "Maths::sq(...)" << '\n';
        return gsq(t);
    }
};

int main()
{
    cout << Maths::sq(42) << '\n';
    cout << Maths::sq(3.14) << '\n';
    return 0;
}

/*
Maths::sq(...)                                                                                                                                                                        
global sq: primary template                                                                                                                                                           
1764                                                                                                                                                                                  
Maths::sq(...)                                                                                                                                                                        
global sq: specialization for double                                                                                                                                                  
9.8596  
*/

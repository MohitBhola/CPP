#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

// a weak functor
struct weak_lesss
{
    template <typename T>
    bool operator()(T const& x, T const& y) const
    {
        return x < y;
    }
};

// a strong functor
// it can share its implementation with a weak functor
template <typename T>
struct lesss : private weak_lesss
{
    auto operator()(T const&x, T const& y) const
    {
        return static_cast<weak_lesss const&>(*this)(x,y);
    }
};

int main()
{
    vector<int> ivec{5,4,3,2,1};
    sort(ivec.begin(), ivec.end(), lesss<int>());
    
    for (auto const& p : ivec)
    {
        cout << p << '\t';
    }
    
    cout << endl;
    
    return 0;
}

/*
1	2	3	4	5
*/

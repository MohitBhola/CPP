#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

template <typename iterator_t, typename object_t>
iterator_t doFind(iterator_t beg, iterator_t end, object_t obj)
{
    return find_if(beg, end, [&](auto const& elem){return elem == obj;});
}

template <typename scalar_t, bool SIGN = true>
struct mimesis_scalar_t
{
    bool operator==(scalar_t const& obj) const
    {
        return SIGN ? obj > 0 : obj < 0;
    }
    
    bool operator!=(scalar_t const& obj) const
    {
        return !(*this == obj);
    }
    
    operator scalar_t() const
    {
        return SIGN ? scalar_t() : -scalar_t();
    }
    
    mimesis_scalar_t<scalar_t, !SIGN> operator!() const
    {
        return mimesis_scalar_t<scalar_t, !SIGN>();
    }
};

template <typename scalar_t, bool SIGN>
bool operator==(scalar_t const& obj, mimesis_scalar_t<scalar_t, SIGN> const& mimesis)
{
    return mimesis == obj;
}

template <typename scalar_t, bool SIGN>
bool operator!=(scalar_t const& obj, mimesis_scalar_t<scalar_t, SIGN> const& mimesis)
{
    return mimesis != obj;
}

int main()
{
    vector<int> ivec{1,2,3,4,5,-1};
    
    if (doFind(ivec.begin(), ivec.end(), 5) != ivec.end())
    {
        cout << "success1" << '\n';
    }
    
    if (find(ivec.begin(), ivec.end(), mimesis_scalar_t<int>()) != ivec.end())
    {
        cout << "success2" << '\n';
    }
    
    if (find(ivec.begin(), ivec.end(), !mimesis_scalar_t<int>()) != ivec.end())
    {
        cout << "success3" << '\n';
    }
    
    return 0;    
}

/*
success1                                                                                                                                                            
success2                                                                                                                                                            
success3 
*/

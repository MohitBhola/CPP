#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

template <typename iterator_t, typename object_t>
iterator_t doFind(iterator_t beg, iterator_t end, object_t obj)
{
    return find_if(beg, end, [&](auto const& elem){return elem == obj;});
}

template <typename scalar_t>
struct positive
{
    bool operator==(scalar_t const& obj) const
    {
        return obj > 0;
    }
    
    bool operator!=(scalar_t const& obj) const
    {
        return !(*this == obj);
    }
    
    operator scalar_t() const
    {
        return scalar_t();
    }
};

template <typename scalar_t>
bool operator==(scalar_t const& obj, positive<scalar_t> const& mimesis)
{
    return mimesis == obj;
}

template <typename scalar_t>
bool operator!=(scalar_t const& obj, positive<scalar_t> const& mimesis)
{
    return mimesis != obj;
}

int main()
{
    vector<int> ivec{1,2,3,4,5};
    if (doFind(ivec.begin(), ivec.end(), 5) != ivec.end())
    {
        cout << "success1" << '\n';
    }
    
    if (find(ivec.begin(), ivec.end(), positive<int>()) != ivec.end())
    {
        cout << "success2" << '\n';
    }
    
    return 0;    
}

/*
success1
success2
*/

#include <iostream>
#include <vector>
#include <string>
using namespace std;

template <typename T>
struct property_size
{
    using value_type = size_t;
    
    value_type operator()(T const& obj) const
    {
        return obj.size();
    }
    
    value_type operator()(T const& obj, value_type const sz)
    {
        obj.resize(sz);
        return obj.size();
    }
};

template <typename iterator_t, typename accessor_t>
typename accessor_t::value_type acc(
    iterator_t beg,
    iterator_t end,
    accessor_t accessor,
    typename accessor_t::value_type& initialValue)
{
    while (beg != end)
    {
        initialValue += accessor(*beg);
        ++beg;
    }
    
    return initialValue;
}

template <typename iterator_t, typename accessor_t>
iterator_t max_elem(
    iterator_t beg,
    iterator_t end,
    accessor_t accessor)
{
    iterator_t result = beg;
    
    while (++beg != end)
    {
        if (accessor(*beg) > accessor(*result))
        {
            result = beg;
        }
    }
    
    return result;
}

int main()
{
    vector<string> svec{"a", "bc", "def"};
    
    vector<string>::size_type sz{};
    acc(svec.begin(), svec.end(), property_size<string>(), sz);
    cout << sz << '\n';
    
    cout << *max_elem(svec.begin(), svec.end(), property_size<string>()) << '\n';
    
    return 0;
}

/*
6
def
*/

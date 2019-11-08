#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>
using namespace std;

template <typename iter1_t, typename end_t, typename iter2_t>
iter2_t doCopy(iter1_t beg, end_t end, iter2_t dst)
{
    while (beg != end)
    {
        *(dst++) = *(beg++);
    }
    
    return dst;
}

template <typename char_t, typename iterator_t = char_t*>
struct c_string_end
{
    operator iterator_t() const
    {
        return iterator_t{};
    }
    
    bool operator==(iterator_t it) const
    {
        return (it == iterator_t{} || *it == 0);
    }
    
    bool operator!=(iterator_t it) const
    {
        return !(*this == it);
    }
};

template <typename char_t, typename iterator_t = char_t*>
bool operator==(iterator_t it, c_string_end<char_t> const& mimesis)
{
    return mimesis == it;
}

template <typename char_t, typename iterator_t = char_t*>
bool operator!=(iterator_t it, c_string_end<char_t> const& mimesis)
{
    return mimesis != it;
}

int main()
{
    char dst[4];
    
    char const* src1 = "xyz";
    (void)doCopy(src1, c_string_end<char const>(), dst);
    cout << dst << '\n';
    
    char const* src2 = "abc";
    (void)doCopy(src2, src2+strlen(src2), dst);
    cout << dst << '\n';

    return 0;    
}

#include <iterator>
#include <type_traits>
#include <utility>
#include <iostream>
#include <vector>
using namespace std;

#define MAX(a,b) ((b) < (a) ? (a) : (b))
#define MIN(a,b) ((b) < (a) ? (b) : (a))

template <typename iterator_t, typename less_t>
auto minmax(iterator_t beg, iterator_t end, less_t less)
{
    pair<iterator_t, iterator_t> result{end, end};

    if (beg == end)
    {
        return result;
    }
    
    result.first = result.second = (beg++);
    
    while (beg != end)
    {
        auto iter1 = beg++;
        auto iter2 = (beg != end) ? beg++ : iter1;
        
        if (less(*iter2, *iter1))
        {
            swap(iter2, iter1);
        }
        
        if (less(*iter1, *result.first))
        {
            result.first = iter1;
        }
        
        if (less(*result.second, *iter2))
        {
            result.second = iter2;
        }
    }
    
    return result;
}

int main()
{
    vector<int> ivec{16,3,1,0,5,6,2,7,4,9,8};
    auto result = minmax(ivec.begin(), ivec.end(), std::less<int>());
    
    cout << *result.first << '\n';   
    cout << *result.second << '\n';   
    
    return 0;
}

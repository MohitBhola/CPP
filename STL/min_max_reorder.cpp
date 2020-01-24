#include <iostream>
#include <vector>

using namespace std;

template <typename iter_t>
auto minmaxcore(iter_t it1, iter_t it2)
{
    if (it1 == it2)
    {
        return it1;
    }
    
    while (it1 < it2) 
    {
        if (*it2 < *it1)
        {
            cout << "swapping " << *it1 << " and " << *it2 << '\n';
            std::swap(*it1, *it2);
        }
        
        ++it1;
        --it2;
    }
    
    if (it1 == it2)
    {
        --it2;
    }
    
    return it2;
}

template <typename iter_t>
void minmax(iter_t beg, iter_t end)
{
    if (beg == end)
    {
        return;
    }
    
    auto it1 = beg;
    auto it2 = end-1;
    
    if (it1 == it2)
    {
        return;
    }
    
    auto it = minmaxcore(it1, it2);
    
    bool doLeft = true;
    bool doRight = true;
    
    if (it == it1)
    {
        doLeft = false;
    }
    
    if ((it+1) == it2)
    {
        doRight = false;
    }
    
    if (doLeft)
    {
        minmax(it1, it+1);
    }
    
    if (doRight)
    {
        minmax(++it, it2+1);   
    }
}

int main()
{
    std::vector<int> ivec{3,5,6,7,3,1,2,7,8,99,0};
    
    minmax(ivec.begin(), ivec.end());
    
    cout << *ivec.begin() << '\n';
    cout << *(ivec.end()-1) << '\n';
    
    return 0;
}

/*
swapping 3 and 0
swapping 3 and 2
swapping 6 and 2
swapping 7 and 6
swapping 8 and 3
swapping 99 and 8
0
99
*/

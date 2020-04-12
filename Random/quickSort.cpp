#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>

using namespace std;

int partition(vector<int>& ivec, int beg, int end)
{
    int len = end - beg + 1;
    
    std::srand(std::time(nullptr));
    int pivot = beg + std::rand() % len;
    
    std::swap(ivec[pivot], ivec[end]);
    
    int small = beg - 1;
    for (int i = beg; i <= end; ++i)
    {
        if (ivec[i] < ivec[end])
        {
            ++small;
            if (small != i)
            {
                std::swap(ivec[small], ivec[i]);    
            }            
        }
    }
    
    ++small;
    if (small != end)
    {
        std::swap(ivec[small], ivec[end]);
    }
    
    return small;
}

void quickSort(vector<int>& ivec, int beg, int end)
{
    if (beg >= end)
    {
        return;
    }
    
    int pivot = partition(ivec, beg, end);
    
    quickSort(ivec, beg, pivot-1);
    quickSort(ivec, pivot+1, end);
}

int main()
{
    vector<int> ivec{12,1,15,4,10,7,20,8};
    
    quickSort(ivec, 0, static_cast<int>(ivec.size())-1);
    
    for (int i : ivec)
    {
        cout << i << '\t';
    }
    cout << '\n';
    
    return 0;
}

/*
1	4	7	8	10	12	15	20	
*/

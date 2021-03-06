#include <iostream>
using namespace std;

/*
** A utility to compute the index of the highest order 1 bit in an unsigned
** Useful for implementing merging traits and type hiding in TMP
*/

constexpr int highest_index_helper(unsigned N)
{
    if (N == 0)
    {    
        return -1;
    }    
 
    int index = -1;       
    unsigned accumulator = 1;
    
    while (accumulator <= N)
    {
        ++index;
        accumulator *= 2;
    } 
    
    return index;
}

int main()
{
    cout << highest_index_helper(0) << '\n'; // 000    
    cout << highest_index_helper(1) << '\n'; // 001
    cout << highest_index_helper(2) << '\n'; // 010
    cout << highest_index_helper(3) << '\n'; // 011
    cout << highest_index_helper(4) << '\n'; // 100
    cout << highest_index_helper(5) << '\n'; // 101
    cout << highest_index_helper(6) << '\n'; // 110
    cout << highest_index_helper(7) << '\n'; // 111
    cout << highest_index_helper(8) << '\n'; // 1000
    
    return 0;
}

/*
-1
0
1
1
2
2
2
2
3
*/

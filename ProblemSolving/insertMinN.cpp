#include <iostream>
using namespace std;

int insertMInN(int M, int N, int i, int j)
{
    // clear off all bits in N from position i till position j
    for (int k = i; k <= j; ++k)
    {
        int tmp = 1 << (k-1);
        N = N & (~tmp);
    }
    
    // shift M up by i-1 bits so as to be ready for insertion into N
    M = M << (i-1);
    
    // do the insertion
    N = N | M;
    
    return N;
}

int main()
{
    int N = 0b00000111;
    int M = 0b00001100;
    
    cout << insertMInN(M, N, 2, 5) << '\n';
    
    return 0;
}

#include <iostream>
using namespace std;

int funcCore(int* pData, int beg, int end)
{
	if (beg == end)
		return beg;
		
	int mid = (beg + end) / 2;
	
	if (pData[mid] >= pData[beg])
	{
		return funcCore(pData, mid + 1, end);
	}
	else
	{	
		return funcCore(pData, beg, mid);
	}
}

int func(int* pData, size_t sz)
{
	return funcCore(pData, 0, sz-1);
}

int main()
{
    int arr1[] = {6,7,8,9,1,2,3,4,5};
    cout << arr1[func(arr1, sizeof(arr1)/sizeof(arr1[0]))] << '\n';
    
    int arr2[] = {6,2,3,4,5};
    cout << arr2[func(arr2, sizeof(arr2)/sizeof(arr2[0]))] << '\n';
    
    return 0;
}

/*
1
2
*/

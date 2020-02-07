#include <iostream>
using namespace std;

int funcCore(int* pData, int beg, int end)
{
	if (beg == end)
	{
		return beg;
	}
		
	int mid = (beg + end) / 2;
			
	if ((mid > beg && pData[mid - 1] < pData[mid]) && (mid < end && pData[mid] > pData[mid + 1]))
	{
		return mid;
	}
	
	if (mid < end && pData[mid] < pData[mid + 1])
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
	return funcCore(pData, 0, sz - 1);
}

int main()
{
	int arr1[] = {1,4,3,2,1};
	cout << arr1[func(arr1, sizeof(arr1)/sizeof(arr1[0]))] << '\n';
	
	int arr2[] = {1,2,3,4,5,6,7,4,3,2,1};
	cout << arr2[func(arr2, sizeof(arr2)/sizeof(arr2[0]))] << '\n';
	
	return 0;
}

/*
4
7
*/

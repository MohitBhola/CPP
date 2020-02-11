/*
Determine whether a given set can be partitioned into subsets such that
the sum of elements in both subsets is same
*/

#include <cstddef>
#include <iostream>
using namespace std;

bool isSubsetSum(int* pData, int numElems, int sum)
{
	if (sum == 0)
		return true;
		
	if (numElems == 0)
		return false;
		
	// if the last element is greator than the sum, ignore it...
	if (pData[numElems - 1] > sum)
		return isSubsetSum(pData, numElems - 1, sum);
		
	//...else, check if the sum could be obtained by including the last element or exclusing the last element!
	return isSubsetSum(pData, numElems - 1, sum) || isSubsetSum(pData, numElems - 1, sum - pData[numElems - 1]);
}

bool isSubset(int* pData, size_t numElems)
{
	size_t sum{};
	
	for (size_t i = 0; i <= numElems - 1; ++i)
		sum += pData[i];

    // if the sum of all the elements is odd, we surely cannot partition the constituent elements into two halves
	if ((sum & 0x1) == 1)
		return false;
		
	return isSubsetSum(pData, numElems, sum / 2);
}

int main()
{
	int arr[] = {1,2,3,4,5,15};
	
	cout << boolalpha << isSubset(arr, sizeof(arr)/sizeof(arr[0]));
	
	return 0;
}

/*
true
*/


/*
Find the number of strings of length 'n' that could be formed with letters
'a', 'b' and 'c', given the constraint that the letter 'b' could only be used once
and the letter 'c' could only be used twice.
*/

#include <iostream>
#include <vector>

using namespace std;

void fooCore(vector<char>& vec, int currIndex, int maxIndex, int& count, int& bCount, int& cCount)
{
	if (currIndex > maxIndex)
	{
		++count;
		return;
	}
	
	// for 'a'
	vec[currIndex] = 'a';
	fooCore(vec, currIndex + 1, maxIndex, count, bCount, cCount);
	
	// for 'b'
	if (bCount == 1)
	{
		vec[currIndex] = 'b';
		fooCore(vec, currIndex + 1, maxIndex, count, --bCount, cCount);
		++bCount;
	}
	
	// for 'c'
	if (cCount == 1 || cCount == 2)
	{
		vec[currIndex] = 'c';
		fooCore(vec, currIndex + 1, maxIndex, count, bCount, --cCount);
		++cCount;
	}
}

int foo(int n)
{
	if (n <= 0)
	{
		return 0;
	}
		
	vector<char> vec;
	vec.resize(n);
	
	int count{};
	int bMaxCount = 1;
	int cMaxCount = 2;
	
	fooCore(vec, 0, n - 1, count, bMaxCount, cMaxCount);
	
	return count;
}

int main()
{
	cout << foo(0) << '\n';
	cout << foo(1) << '\n';
	cout << foo(2) << '\n';
	cout << foo(3) << '\n';

	return 0;
}

/*
0
3
8
19
*/

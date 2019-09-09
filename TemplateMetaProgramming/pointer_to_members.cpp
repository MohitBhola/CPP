#include <utility>
#include <iostream>
using namespace std;

// global pointer-to-members
// help in providing better names
using IDValue = std::pair<int, double>;
int IDValue::*pID = &IDValue::first; // a better name for the first component of a pair of int and double 
double IDValue::*pValue = &IDValue::second; // a better name for the second component of a pair of int and double

int main(int argc, char **argv)
{
    IDValue aIDValue{42, 43.1};
    
    cout << aIDValue.*pID << '\n'; // note usage syntax
    cout << aIDValue.*pValue << '\n'; // note usage syntax
    
	return 0;
}

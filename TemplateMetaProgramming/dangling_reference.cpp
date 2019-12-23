#include <cstring>
#include <iostream>

using namespace std;

template <typename T>
const T& max (const T& a, const T& b)
{
    return b < a ? a : b;
}

// WARNING
// this function returns a temporary
// do not attempt to bound the return value of this function to a reference parameter
const char* max (const char* a, const char* b)
{
    return std::strcmp(b,a) < 0 ? a : b; 
}

template <typename T>
const T& max (const T& a, const T& b, const T& c)
{
    // both the calls to max resolve to the overload intended for C_Strings
    // that is, both the parameters and the return type is a copy of the pointer to a C_String
    // specifically, the return value is a temporary in the stack frame of this function
    // 
    // unfortunately, this function returns a reference to such a temporary, and thus leads to a crash!
    return max(max(a,b),c);
}

int main(int argc, char **argv)
{
	const char* s1 = "c";
    const char* s2 = "b";
    const char* s3 = "a";
    
    auto result = max(s1,s2,s3);
    cout << "Max: " << result << endl;
    
	return 0;
}

/*
OUTPUT
Segmentation fault!!!
*/

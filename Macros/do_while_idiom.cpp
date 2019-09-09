#include <iostream>
using namespace std;

// the do {...} while (false) idiom 
// allows us to put local code inside a block and terminate the directive with a semicolon
// this prevents numerous *accidental* usage errros

#define MXT_SORT2(a,b) \
do \ 
{ \
    if ((b) < (a)) \
    { \
        std::swap((a),(b)); \
    } \
} while(false) \

#define MXT_SORT3(x,y,z) \
do \
{ \
    MXT_SORT2(x,y); \
    MXT_SORT2(x,z); \
    MXT_SORT2(y,z); \
} while (false) \    
    
int main(int argc, char **argv)
{
    int a = 5, b = 3, c = 2;
    MXT_SORT3(a,b,c);
    
    cout << a << b << c << '\n';
    
	  return 0;
}

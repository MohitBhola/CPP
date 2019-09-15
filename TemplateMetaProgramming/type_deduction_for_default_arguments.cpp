#include <string>
using namespace std;

// if we want to provide a default value for a function template call parameter
// that depends upon a template parameter, we also need to provide a default argument 
// for that template parameter
template <typename T = string>
void foo(T = "")
{}

int main(int argc, char **argv)
{
    foo(1); // foo<int> instantiated
    foo();  // foo<string> instantiated
    
    return 0;
}

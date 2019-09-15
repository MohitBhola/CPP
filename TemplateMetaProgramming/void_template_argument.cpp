#include <iostream>
using namespace std;

template <typename T>
void foo(T* ptr)
{}

int main(int argc, char **argv)
{
    // void is a valid template argument, 
    // provided the resulting code is valid
    void* pVoid = nullptr;
    foo(pVoid);
    
    return 0;
}

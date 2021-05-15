#include <iostream>
#include <memory>
using namespace std;

template<
    typename T>
void SuppressWarning(T const&) {}

template<
    typename T, 
    typename A = T*>
class Singleton
{
    using AccessType = A;
    static AccessType sSingleton; // declaration
    
public:

    Singleton()
    {
        static bool init = [] () -> bool {
            sSingleton = T::Create();
            return true;
        }();
        
        SuppressWarning(init);
    }

    AccessType operator->() {return sSingleton;}
    AccessType const operator->() const {return sSingleton;}
};

template<
    typename T,
    typename A>
A Singleton<T,A>::sSingleton; // definition

struct MyClass1
{
    int val = 41;
    
    static MyClass1* Create()
    {
        return new MyClass1;
    }
};

struct MyClass2
{
    int val = 42;
    
    static MyClass2* Create()
    {
        return new MyClass2;
    }
};

struct MyClass3;
using MyClass3Ptr = std::shared_ptr<MyClass3>;
struct MyClass3
{
    int val = 43;
    
    static MyClass3Ptr Create()
    {
        return MyClass3Ptr(new MyClass3);
    }
};

int main(void)
{
    Singleton<MyClass1> s1{};
    Singleton<MyClass2> s2{};
    Singleton<MyClass3, MyClass3Ptr> s3{};
    
    cout << s1->val << '\n';
    cout << s2->val << '\n';
    cout << s3->val << '\n';
    
    return 0;
};

/*
41
42
43
*/

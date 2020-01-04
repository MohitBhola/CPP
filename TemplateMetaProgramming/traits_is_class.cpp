#include <iostream>
using namespace std;

template <typename T>
class IsClassHelper
{
    template <typename U, typename = decltype(declval<int U::*>())>
    static true_type test(U*);
    
    template <typename U>
    static false_type test(...);
    
public:

    using type = decltype(test<T>(nullptr));
};

template <typename T>
struct IsClass : IsClassHelper<T>::type
{};

// alternate solution based on SFINAE'ing out partial specialization
/*
template <typename T, typename = void_t<>>
struct IsClass : public false_type
{};

template <typename T>
struct IsClass<T, void_t<int T::*>> : public true_type
{};
*/

struct Foo
{};

int main()
{
    cout << boolalpha << IsClass<Foo>::value << '\n';
    cout << boolalpha << IsClass<int>::value << '\n';
    return 0;
}

/*
true
false
*/

#include <iostream>
using namespace std;

/*
template <typename T, typename = void_t<>>
struct IsClass : public false_type
{};

template <typename T>
struct IsClass<T, void_t<int T::*>> : public true_type
{};
*/

template <typename T>
struct YES
{};

using NO = char;

template <typename T>
class IsClassHelper
{
    template <typename U>
    static YES<decltype(declval<int U::*>())> test(U*);
    
    template <typename U>
    static NO test(...);
    
    template <typename U>
    static true_type cast(YES<U>);
    
    static false_type cast(NO);
    
public:

    using type = decltype(cast(test<T>(nullptr)));
};

template <typename T>
struct IsClass : IsClassHelper<T>::type
{};

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

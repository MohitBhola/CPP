#include <iostream>
#include <vector>
#include <utility>
using namespace std;

struct fake_incrementable
{
    // a non-explicit universal ctor
    template <typename T>
    fake_incrementable(T);
};

fake_incrementable operator++(fake_incrementable);

true_type test(fake_incrementable);

template <typename T>
static false_type test(T);

// the key thing to note here is the expression test(++declval<T&>())
// if the preincrement operator actually exists, the second overload gets picked up
// however, if the preincrement operator doesn't exists, an implicit conversion of T to
// fake_incrementable happens, for which, the preincrement operator 
// has been declared (at the namespace level, not as a member!)
// thus, the first overload gets picked up
template <typename T>
struct has_preincrement
{
    static const bool value = is_same<decltype(test(++declval<T&>())), false_type>::value;
};

// alternate solution based on SFINAE'ing out partial specialization
/*
template <typename T, typename = void_t<>>
struct has_preincrement : false_type
{};

template <typename T>
struct has_preincrement<T, void_t<decltype(++declval<T&>())>> : true_type
{};
*/

int main()
{
    cout << has_preincrement<int>::value << '\n';
    cout << has_preincrement<vector<int>::iterator>::value << '\n';
    cout << has_preincrement<vector<int>>::value << '\n';
    
    return 0;
}

/*
1
1
0
*/

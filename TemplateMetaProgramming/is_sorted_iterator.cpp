#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
using namespace std;

template <typename iterator_t, typename value_t = typename iterator_traits<iterator_t>::value_type>
struct is_sorted_iterator
{
    static constexpr bool value = 
        disjunction_v<
            is_same<iterator_t, typename std::set<value_t>::iterator>,
            is_same<iterator_t, typename std::set<value_t>::const_iterator>,
            is_same<iterator_t, typename std::multiset<value_t>::iterator>,
            is_same<iterator_t, typename std::multiset<value_t>::const_iterator>>;
};

template <typename iterator_t, typename T1, typename T2>
struct is_sorted_iterator<iterator_t, pair<const T1, T2>>
{
    static constexpr bool value = 
        disjunction_v<
            is_same<iterator_t, typename std::map<T1, T2>::iterator>,
            is_same<iterator_t, typename std::map<T1, T2>::const_iterator>,
            is_same<iterator_t, typename std::map<T1 const, T2>::iterator>,
            is_same<iterator_t, typename std::map<T1 const, T2>::const_iterator>,
            is_same<iterator_t, typename std::multimap<T1, T2>::iterator>,
            is_same<iterator_t, typename std::multimap<T1, T2>::const_iterator>,
            is_same<iterator_t, typename std::multimap<T1 const, T2>::iterator>,
            is_same<iterator_t, typename std::multimap<T1 const, T2>::const_iterator>>;
};

int main()
{
    cout << boolalpha << is_sorted_iterator<set<int>::iterator>::value << '\n';
    cout << boolalpha << is_sorted_iterator<set<int>::const_iterator>::value << '\n';
    
    cout << boolalpha << is_sorted_iterator<multiset<int>::iterator>::value << '\n';
    cout << boolalpha << is_sorted_iterator<multiset<int>::const_iterator>::value << '\n';
    
    cout << boolalpha << is_sorted_iterator<map<int, int>::iterator>::value << '\n';
    cout << boolalpha << is_sorted_iterator<set<int, int>::const_iterator>::value << '\n';
    
    cout << boolalpha << is_sorted_iterator<multimap<int, int>::iterator>::value << '\n';
    cout << boolalpha << is_sorted_iterator<multimap<int, int>::const_iterator>::value << '\n';
    
    cout << boolalpha << is_sorted_iterator<unordered_map<int, int>::iterator>::value << '\n';
    cout << boolalpha << is_sorted_iterator<unordered_map<int, int>::const_iterator>::value << '\n';
    
    cout << boolalpha << is_sorted_iterator<vector<int>::iterator>::value << '\n';
    cout << boolalpha << is_sorted_iterator<vector<int>::const_iterator>::value << '\n';
    
    return 0;    
}

/*
true
true
true
true
true
true
true
true
false
false
false
false
*/

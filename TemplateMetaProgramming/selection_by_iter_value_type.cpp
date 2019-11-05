#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <vector>
using namespace std;

template <typename T>
struct instance_of
{
    using type = T;
};

template <typename iterator_t>
void func(iterator_t b, iterator_t e)
{
    using value_type = typename iterator_traits<iterator_t>::value_type;
    
    func_core(b, e, instance_of<value_type>());
}

template <typename iterator_t, typename T1, typename T2>
void func_core(iterator_t b, iterator_t e, instance_of<pair<T1 const, T2>>)
{
    cout << "func_core(...) called for a map\n";
}

template <typename iterator_t, typename T>
void func_core(iterator_t b, iterator_t e, instance_of<T>)
{
    cout << "func_core(...) called for a non-map\n";
}

int main()
{
    map<int, int> imap{{1,1},{2,2}};
    set<int> iset{1,2};
    vector<int> ivec{1,2};
    
    func(imap.begin(), imap.end());
    func(iset.begin(), iset.end());
    func(ivec.begin(), ivec.end());
    
    return 0;   
}

/*
func_core(...) called for a map
func_core(...) called for a non-map
func_core(...) called for a non-map
*/


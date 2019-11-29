#include <iostream>
#include <type_traits>
using namespace std;

// an iterator need not be necessarily coupled with an underlying container/data-source
// it just needs to implement a minimal interface
// in this example, we implement a simple iterator that progressively generates sequential values of any integral type
// it quallifies to be usable in the fancy C++11 range-based for loop
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
class num_iterator
{
    T i;
    
public:

    explicit num_iterator(int position = 0) : i(position) {}
    
    T operator*() const {return i;}
    
    num_iterator& operator++() 
    {
        ++i;
        return *this;
    }
    
    bool operator!=(num_iterator const& other) const
    {
        return i != other.i;
    }
};

// num_range is a class that *serves* num_iterators
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
class num_range
{
    T _beg;
    T _end;
    
public:

    explicit num_range(T b, T e) : _beg(b), _end(e) {}
    
    auto begin() const
    {
        return num_iterator<T>(_beg);
    }
    
    auto end() const
    {
        return num_iterator<T>(_end);
    }
};

int main()
{
    for (int i : num_range(1, 10))
    {
        cout << i << '\n';
    }
    
    return 0;
}

/*
1
2
3
4
5
6
7
8
9
*/

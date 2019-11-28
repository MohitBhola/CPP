#include <vector>
#include <iostream>
#include <algorithm>
#include <array>
using namespace std;

// the 2nd non-type template parameter specifies the initial capacity of the special vector; it is an implementation detail
// but still, it renders two special vectors of the same type but with different initial capacities as different types
// to *ignore* this implementation detail, some member functions of the class template special_vector need to be promoted to templates
// for example, we may need to provide an equality operator that can compare any two special vector types regardless of their initial capacity
template <typename T, size_t N = 0>
class special_vector
{
    vector<T> vec{};

public:

    special_vector() : vec(N) {}

    // disregards K
    // that is, compares two special vectors regardless of their initial capacity
    template <size_t K>
    bool operator==(special_vector<T, K> const& other) const
    {
        if (size() != other.size())
        {
            return false;
        }
        
        for (size_t i = 0; i < N; ++i)
        {
            if (vec[i] != other[i])
            {
                return false;
            }
        }
        
        return true;
    }
    
    auto size() const
    {
        return vec.size();
    }
    
    int const& operator[](size_t i) const
    {
        return vec[i];
    }
    
    int& operator[](size_t i) 
    {
        return vec[i];
    }
    
    void print()
    {
        for (auto const& elem : vec)
        {
            cout << elem << '\n';
        }
    }
    
    template <typename U>
    void push_back(U&& elem)
    {
        vec.push_back(forward<U>(elem));
    }
};

int main()
{
    special_vector<int, 1> avec1{};
    special_vector<int, 2> avec2{};
    
    cout << boolalpha << (avec1 == avec2) << '\n';
    
    avec1.push_back(0);
    
    cout << boolalpha << (avec1 == avec2) << '\n';
    
    avec1.print();
    cout << "===" << '\n';
    avec2.print();
    
    return 0;
}

/*
false
true
0
0
===
0
0
*/

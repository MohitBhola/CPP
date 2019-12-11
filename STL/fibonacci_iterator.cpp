#include <iostream>
#include <sstream>
using namespace std;

// another example demonstrating that an iterator need not move along elements of an underlying container
// that is, there need not be an underlying container
// it just needs to conform to a *minimal* interface
// specifically, if it provides for increment/dereference/comparison(!=), it can be employed in algorithms
class FibonacciIterator
{
    size_t index {0};
    size_t a {0};
    size_t b {1};
    size_t val {0};
    
public:

    explicit FibonacciIterator(size_t i = 0) : index(i) {}
    
    // dereference
    // b is the current 
    size_t operator*() const 
    {
        return val;
    }
    
    // increment
    // moves the internal state to the next fibonacci number
    // also updates the index
    FibonacciIterator& operator++()
    {
        if (index == 0)
        {
            val = 1;
        }
        else 
        {
            val = a + b;
            size_t old_b = b;
            b += a;
            a = old_b;
            ++index;
        }
        
        return *this;   
    }
    
    // comparison operator against an *end* iterator
    friend bool operator!=(FibonacciIterator const& it1, FibonacciIterator const& it2) 
    {
        return it1.index != it2.index;
    }
};

class FibonacciRange
{
    size_t b;
    size_t e;

public:

    FibonacciRange(size_t begin, size_t end) : b(begin), e(end) 
    {
        if (b > e)
        {
            std::ostringstream oss{};
            oss << "Invalid Fibonacci Range: [" << begin << "," << end << ")";
            throw std::runtime_error(oss.str().c_str());
        }
    }
    
    auto begin() { return FibonacciIterator(b); }
    auto end() { return FibonacciIterator(e); }
};

int main()
{
    for (auto num : FibonacciRange(1, 10))
    {
        cout << num << '\t';
    }
    
    cout << '\n';
 
    /*
    The following block would emit an unhandled exception, and thus crash the program
    for (auto num : FibonacciRange(11, 10))
    {
        cout << num << '\t';
    }
    */
    
    cout << '\n';

    return 0;
}

/*
0	1	2	3	5	8	13	21	34	
*/

#include <iostream>
using namespace std;

/*
 * _Usually_, the STL algorithms and the range based for loops assume that the begin and end positions of the iterations are known in advance.
 *
 * But this isn't always true. 
 * For example, to provide iterators over the range of a C-String , we need to make a single pass over it to obtain the end iterator.
 * Also, while capturing user input from std::cin (as the user is still typing), std::istream_iterator also faces a similar conundrum.
 *
 * An iterator sentinel is a technique to the rescue in such situations. 
 * The crux of this technique is to have _different_ iterator types for the begin and end positions of the iteration.
 * The end iterator would just be a sentinel; it has an empty implementation.
 * The _normal_ iterator would provide for usual dereference/increment operations, but also a comparison operator against the sentinel.
 */
 
class cstr_sentinel {};

class cstr_iterator
{
    char const* cstr{nullptr};
    
public:

    explicit cstr_iterator(char const* ptr) : cstr(ptr) {}
    
    // dereference
    char operator*() 
    {
        return *cstr;
    }
    
    // increment
    cstr_iterator& operator++()
    {
        ++cstr;
        return *this;
    }
    
    // comparison against the sentinel
    // just check whether we have reached the end of the C-String
    bool operator!=(cstr_sentinel) const
    {
        return cstr != nullptr && *cstr != '\0';
    }
};

// a range class that serves the _normal_ (begin) and _sentinel_ (end) iterators
class cstr_range
{
    char const* cstr{nullptr};
    
public:

    explicit cstr_range(char const* p) : cstr(p) {}
    
    auto begin() const { return cstr_iterator(cstr); }
    auto end() const { return cstr_sentinel(); }
};

int main()
{
    char const* cstr = "Hello World!";
    for (char ch : cstr_range(cstr))
    {
        cout << ch;
    }
    cout << '\n';

    return 0;    
}

/*
Hello World!
*/

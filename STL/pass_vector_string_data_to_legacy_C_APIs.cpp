#include <vector>
#include <iostream>
#include <string>
using namespace std;

constexpr size_t sz = 10;

template<
    typename T>
size_t fill(T* ptr, size_t numElements, T const& initialValue)
{
    T val = initialValue;

    // fill only half of the array
    for (size_t i = 0; i < numElements / 2; ++i)
    {
        *ptr++ = initialValue++;
    }

    // inform that we filled only half of the array
    return numElements / 2;
}

int main()
{
    // legacy C APIs traffic in raw arrays
    // they usually also require the size of the array, but C-style strings (ie, arrays of char) are demarkend by '\0'
    
    // the most reliable, portable way to pass a vector's data to a legacy C API is: &vec[0]
    // in particular, *don't* use: vec.begin(), thinking that iterators into vectors are pointers anyways
    // that's not always true; iterators are iterators, not pointers
    
    // here, we create a vector of integers with some initial size
    // and then request a legacy C API to *overwrite* it with useful values
    // note that such a legacy API must not alter the size of the array
    // it should just overwrite useful values
    // if it did alter the size of the array, the vector would not know about it and would become corrupt
    
    // also, if there are any semantic constraints, such as *the vector shall always remain sorted*
    // either the legacy C API or the calling code should assume the responsibility to uphold such constraints
    vector<int> ivec(sz);
    ivec.resize(fill(&ivec[0], ivec.size(), 42));

    cout << "ivec Size: " << ivec.size() << '\n';
    cout << "ivec Capacity: " << ivec.capacity() << '\n';

    for (auto const& elem : ivec)
    {
        cout << elem << '\t';
    }

    cout << '\n';

    // to have a legacy C API provide useful data to string, have it first populate a vector<char>
    // which could then be used to populate the string
    // as only a vector provides the guarantee that its underlying data in contiguous
    vector<char> cvec(sz);
    cvec.resize(fill(&cvec[0], cvec.size(), 'a'));
    string str(cvec.begin(), cvec.end());
    cout << "str Size: " << str.size() << '\n';
    cout << "str Capacity: " << str.capacity() << '\n';
    cout << "str: " << str << '\n';

    return 0;
}

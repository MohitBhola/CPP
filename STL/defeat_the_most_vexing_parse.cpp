#include <vector>
#include <iterator>
#include <iostream>
#include <string>
using namespace std;

int main()
{
    // expectation: a vector of strings constructed via range assignment using istream iterators operating on a stream of strings
    // reality: a function ivec that returns a vector of strings, taking two parameters
    // the first parameter is of the type istream_iterator<string> named cin (the parens around the parameter name are ignored)
    // the second parameter is of the type pointer to a function taking nothing and returning a istream_iterator<string>
    
    //std::vector<string> ivec(istream_iterator<string>(cin), istream_iterator<string>());
    
    // how to defeat the most vexing parse: 
    // 1. use uniform initialization syntax (since C++11)
    // 2. surround arguments in parens
    
    //std::vector<string> ivec{istream_iterator<string>(cin), istream_iterator<string>()};
    std::vector<string> ivec((istream_iterator<string>(cin)), (istream_iterator<string>()));

    for (auto const& elem : ivec)
    {
        cout << elem << '\t';
    }

    cout << '\n';

    return 0;
}

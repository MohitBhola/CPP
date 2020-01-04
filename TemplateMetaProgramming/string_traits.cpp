#include <iostream>
#include <string>
using namespace std;

template <typename string_t>
struct string_traits;

// partial specialization for string types minted from the template std::basic_string
template <typename char_t>
struct string_traits<std::basic_string<char_t>>
{
    using char_type = char_t;   
    using const_iterator = typename std::basic_string<char_t>::const_iterator;
    using arg_type = std::basic_string<char_t> const&;
    
    static const_iterator begin(arg_type s)
    {
        return s.begin();
    }
    
    static const_iterator end(arg_type s)
    {
        return s.end();
    }
    
    static bool is_end_of_string(const_iterator it, arg_type s)
    {
        return it == s.end();
    }
};

// full specialization for C-Strings
template <>
struct string_traits<char const*>
{
    using char_type = char;
    using const_iterator = char const*;
    using arg_type = char const*;
    
    static char const* begin(arg_type s)
    {
        return s;
    }
    
    static char const* end(arg_type s)
    {
        return 0; // constant time
    }
    
    static bool is_end_of_string(const_iterator it, arg_type UNUSED)
    {
        return it == 0 || *it == '\0'; // constant time 
    }
};

// with the above specializations in place, we can write generic code like this
// this would *work* for library string types as well as C-Strings
template <typename string_t>
void processAllChars(string_t const& s)
{
    using traits = string_traits<string_t>;
    
    auto beg = traits::begin(s);
    while (!traits::is_end_of_string(beg, s))
    {
        cout << *beg++;
    }
}

int main()
{
    string s1("Hello");
    processAllChars(s1);
    
    cout << '\n' << "=====" << '\n';
    
    char const* s2 = "World";
    processAllChars(s2);
    
    return 0;
}

/*
Hello
=====
World
*/

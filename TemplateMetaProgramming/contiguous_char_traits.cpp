#include <clocale>
#include <iostream>
using namespace std;

template <
    typename char_t,
    char C0 = 0,
    char C1 = 0,
    char C2 = 0,
    char C3 = 0,
    char C4 = 0,
    char C5 = 0,
    char C6 = 0,
    char C7 = 0,
    char C8 = 0,
    char C9 = 0
>
struct is_contiguous
{
    static const bool value = ((C0+1)==C1) && is_contiguous<char_t, C1, C2, C3, C4, C5, C6, C7, C8, C9>::value;
};

template <char C>
struct is_contiguous<char, C>
{
    static const bool value = true;
};

struct ascii
{
    static const bool value_lowerc = 
        is_contiguous<char, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'>::value
        && is_contiguous<char, 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'>::value
        && is_contiguous<char, 'u', 'v', 'w', 'x', 'y', 'z'>::value;
    
    static const bool value_upperc = 
        is_contiguous<char, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'>::value
        && is_contiguous<char, 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'>::value
        && is_contiguous<char, 'U', 'V', 'W', 'X', 'Y', 'Z'>::value;
    
    static const bool value_09 = is_contiguous<char, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>::value;
    
    static const bool value = value_lowerc && value_upperc && value_09;
};

template <typename T, T lower, T upper>
inline bool is_between(const T c)
{
    return !(c<lower) && !(c>upper);
}

struct ascii_traits
{
    using char_t = char;
    
    static inline bool isupper(char_t const c)
    {
        cout << "ascii_traits::isupper" << '\n';
        return is_between<char_t,'A','Z'>(c);    
    };
    
    static inline bool islower(char_t const c)
    {
        cout << "ascii_traits::islower" << '\n';
        return is_between<char_t,'a','z'>(c);    
    };
    
    static inline bool isalpha(char_t const c)
    {
        cout << "ascii_traits::isalpha" << '\n';
        return islower(c) || isupper(c);
    };
    
    static inline bool isdigit(char_t const c)
    {
        cout << "ascii_traits::isdigit" << '\n';
        return is_between<char_t,'0','9'>(c);
    };
    
    static inline char_t tolower(char_t const c)
    {
        cout << "ascii_traits::tolower" << '\n';
        return islower(c) ? c : c - 'A' + 'a';
    };
    
    static inline char_t toupper(char_t const c)
    {
        cout << "ascii_traits::toupper" << '\n';
        return isupper(c) ? c : c - 'a' + 'A';
    };
};

template <typename char_t>
struct stdchar_traits
{    
    static inline bool isupper(char_t const c)
    {
        cout << "stdchar_traits::isupper" << '\n';
        return std::isupper(c, locale());
    };
    
    static inline bool islower(char_t const c)
    {
        cout << "stdchar_traits::islower" << '\n';
        return std::islower(c, locale());
    };
    
    static inline bool isalpha(char_t const c)
    {
        cout << "stdchar_traits::isalpha" << '\n';
        return std::isalpha(c, locale());
    };
    
    static inline bool isdigit(char_t const c)
    {
        cout << "stdchar_traits::isdigit" << '\n';
        return std::isdigit(c, locale());
    };
    
    static inline char_t tolower(char_t const c)
    {
        cout << "stdchar_traits::tolower" << '\n';
        return std::tolower(c, locale());
    };
    
    static inline char_t toupper(char_t const c)
    {
        cout << "stdchar_traits::toupper" << '\n';
        return std::toupper(c, locale());
    };
};

struct standard{};
struct fast{};

template <typename char_t, typename charset_t = standard>
struct chartraits : stdchar_traits<char_t>
{};

template <>
struct chartraits<char, fast> : conditional<ascii::value, ascii_traits, stdchar_traits<char>>::type
{};

int main()
{
    using traits_standard_t = chartraits<char>;
    
    cout << boolalpha << traits_standard_t::isupper('A') << '\n';
    cout << boolalpha << traits_standard_t::islower('a') << '\n';
    cout << boolalpha << traits_standard_t::isdigit('0') << '\n';
    
    using traits_fast_t = chartraits<char, fast>;
    
    cout << boolalpha << traits_fast_t::isupper('A') << '\n';
    cout << boolalpha << traits_fast_t::islower('a') << '\n';
    cout << boolalpha << traits_fast_t::isdigit('0') << '\n';
    
    return 0;
}

/*
stdchar_traits::isupper
true
stdchar_traits::islower
true
stdchar_traits::isdigit
true
ascii_traits::isupper
true
ascii_traits::islower
true
ascii_traits::isdigit
true
*/




#include <iostream>
#include <initializer_list>
using namespace std;

// Rules:

// 1. A sequence of statements separated by commas is evaluated left-to-right.
//    Furthermore, the results of the first statement are fully evaluated before the next statement is executed.

// 2. Braced initializer lists have the same sequenced guarantees that the comma operator gives us.

// The technique:
// Create a temporary std::initializer_list, and try to initialize it with a repeated pattern like: ((do_something), 0)...
// This would action the do_something, discard its result and subsequently return a 0 which would be used as an initializer for the initializer list.
// As per the aforementioned rules, the various actions (do_something) are sequenced. 
// That is, they happen sequentially, and the results of an action are fully evaluated before the next action is executed.
template <typename... T>
void print(T const&... ts)
{
    (void)initializer_list<int>{((cout << ts << '\n'), 0)...};
}

template <typename... T>
void csv(T const&... ts)
{
    bool comma = false;
    (void)initializer_list<bool>{((cout << (comma == false ? "" : ",") << ts), comma = true)...};
    cout << '\n';
}

template <typename... T>
bool and_all(T const&... ts)
{
    bool result = true;
    (void)initializer_list<bool>{((result = result && ts), result)...};
    return result;
}

template <typename... Ts>
auto sum_all(Ts const&... ts)
{
    using result_type = std::common_type_t<Ts...>;
    result_type result = {};
    (void)initializer_list<int>{((result += ts), 0)...};
    return result;
}

template <typename T, typename... Ts>
auto min(T const& t, Ts const&... ts)
{
    using result_type = std::common_type_t<T, Ts...>;
    result_type result = static_cast<T>(t);
    (void)initializer_list<int>{((result = (result < ts ? result : static_cast<result_type>(ts))), 0)...};
    return result;
}

int main()
{
    print(1, 2.2, "abc");
    csv(1, 2.2, "abc");
    cout << and_all(1,2,0) << '\n';
    cout << and_all(1,2,3) << '\n';
    cout << sum_all(1,2,3,4,5) << '\n';
    cout << min(3,4,1.1,2,5) << '\n';
    
    return 0;
}

/*
1
2.2
abc
1,2.2,abc
0
1
15
1.1
*/

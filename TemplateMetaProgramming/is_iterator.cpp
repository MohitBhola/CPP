#include <iostream>
#include <type_traits>
#include <iterator>
#include <vector>
#include <map>
using namespace std;

#define DEFINE_HAS_TYPE(MemType) \
template <typename T, typename = void_t<>> \
struct has_type_##MemType : false_type \
{}; \
template <typename T> \
struct has_type_##MemType<T, void_t<typename T::MemType>> : true_type \
{}

DEFINE_HAS_TYPE(difference_type);
DEFINE_HAS_TYPE(value_type);
DEFINE_HAS_TYPE(pointer);
DEFINE_HAS_TYPE(reference);
DEFINE_HAS_TYPE(iterator_category);

template <typename T>
struct is_iterator
{
    static constexpr bool value = conjunction_v<
                                    has_type_difference_type<T>,
                                    has_type_value_type<T>,
                                    has_type_pointer<T>,
                                    has_type_reference<T>,
                                    has_type_iterator_category<T>>;
};

template <typename T>
struct is_iterator<T*>
{
    static constexpr bool value = true;
};

struct Foo
{};

int main()
{
    cout << boolalpha << is_iterator<vector<int>::iterator>::value << '\n';
    cout << boolalpha << is_iterator<vector<int>::const_iterator>::value << '\n';
    cout << boolalpha << is_iterator<Foo*>::value << '\n';
    
    // note that a std::map has all the typedefs required from a standard iterator
    // *except* iterator_category
    cout << boolalpha << is_iterator<std::map<int, int>>::value << '\n';
    
    return 0;
}

/*
true
true
true
false
*/




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
struct is_iterator : bool_constant<
                        conjunction_v<
                            has_type_difference_type<T>,
                            has_type_value_type<T>,
                            has_type_pointer<T>,
                            has_type_reference<T>,
                            has_type_iterator_category<T>>>
{};

template <typename T>
struct is_iterator<T*> : true_type
{};

template <typename T1, typename T2>
struct is_mutable_iterator_helper : false_type
{};

template <typename T>
struct is_mutable_iterator_helper<T&, T> : true_type
{};

template <typename T, bool IS_ITERATOR = is_iterator<T>::value>
class is_mutable_iterator
{
    using value_t = typename iterator_traits<T>::value_type;
    using reference_t = typename iterator_traits<T>::reference;
    
public:

    static constexpr bool value = disjunction_v<
                                    is_mutable_iterator_helper<reference_t, value_t>,
                                    bool_constant<is_convertible_v<reference_t, value_t> && !is_convertible_v<value_t, reference_t>>>;
};

template <typename T>
class is_mutable_iterator<T, false>
{
public:
    static constexpr bool value = false;
};

struct Foo
{};

int main()
{
    cout << boolalpha << is_mutable_iterator<int>::value << '\n';
    
    using ivec_iterator_t = vector<int>::iterator;
    cout << boolalpha << is_mutable_iterator<ivec_iterator_t>::value << '\n';
    
    using ivec_const_iterator_t = vector<int>::const_iterator;
    cout << boolalpha << is_mutable_iterator<ivec_const_iterator_t>::value << '\n';
    
    using imap_iterator_t = map<int, int>::iterator;
    cout << boolalpha << is_mutable_iterator<imap_iterator_t>::value << '\n';
    
    using imap_const_iterator_t = map<int, int>::const_iterator;
    cout << boolalpha << is_mutable_iterator<imap_const_iterator_t>::value << '\n';
    
    return 0;
}

/*
false
true
false
true
false
*/

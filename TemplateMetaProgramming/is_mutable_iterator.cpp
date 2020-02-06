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

template <typename iter_t>
struct is_mutable_iterator_helper
{
    using val_t = typename iterator_traits<iter_t>::value_type;
    
    // expression SFINAE with decltype
    // an iterator is mutable if we can assign to the result of dereferencing it
    template <typename I, typename V>
    static auto test(void*) -> decltype(*std::declval<I>() = std::declval<V>(), true_type());
    
    template<typename I, typename V>
    static false_type test(...);
    
    using type = decltype(test<iter_t, val_t>(nullptr));
};

// if iter_t is indeed an iterator, proceed for the real test
template <typename iter_t, bool IS_ITERATOR = is_iterator<iter_t>::value>
struct is_mutable_iterator : is_mutable_iterator_helper<iter_t>::type
{};

// if iter_t isn't even an iterator
template <typename T>
struct is_mutable_iterator<T, false> : std::false_type
{};

struct Foo
{};

int main()
{
    cout << boolalpha << is_mutable_iterator<int>::value << '\n';
    cout << "=====" << '\n';
    
    cout << boolalpha << is_mutable_iterator<Foo>::value << '\n';
    cout << "=====" << '\n';
    
    using ivec_iterator_t = vector<int>::iterator;
    cout << boolalpha << is_mutable_iterator<ivec_iterator_t>::value << '\n';
    cout << "=====" << '\n';
    
    using ivec_const_iterator_t = vector<int>::const_iterator;
    cout << boolalpha << is_mutable_iterator<ivec_const_iterator_t>::value << '\n';
    cout << "=====" << '\n';
    
    using imap_iterator_t = map<int, int>::iterator;
    cout << boolalpha << is_mutable_iterator<imap_iterator_t>::value << '\n';
    cout << "=====" << '\n';
    
    using imap_const_iterator_t = map<int, int>::const_iterator;
    cout << boolalpha << is_mutable_iterator<imap_const_iterator_t>::value << '\n';
    
    return 0;
}

/*
false
=====
false
=====
true
=====
false
=====
false
=====
false
*/

#include <iostream>
#include <cstddef>
#include <algorithm>
#include <map>
using namespace std;

template <typename iter_t, typename diff_t = ptrdiff_t>
class iterator_facade
{
protected:

    iterator_facade() = default;
    ~iterator_facade() noexcept = default;
    
    iter_t& true_this()
    { return static_cast<iter_t&>(*this); }
    
    iter_t const& true_this() const
    { return static_cast<iter_t const&>(*this); }
    
public:

    iter_t& operator++()
    { ++true_this().base(); return true_this(); }
    
    iter_t& operator--()
    { --true_this().base(); return true_this(); }
    
    iter_t operator++(int)
    {
        iter_t result(true_this());
        ++(*this);
        return result;
    }
    
    iter_t operator--(int)
    {
        iter_t result(true_this());
        --(*this);
        return result;
    }
    
    iter_t operator+(diff_t diff)
    {
        iter_t result(true_this());
        result.base() += diff;
        return result;
    }
    
    iter_t operator-(diff_t diff)
    {
        iter_t result(true_this());
        result.base() -= diff;
        return result;
    }
    
    diff_t operator-(iterator_facade const& other) const
    { return true_this().base() - other.true_this().base(); }
    
    diff_t operator<(iterator_facade const& other) const
    { return true_this().base() < other.true_this().base(); }
    
    diff_t operator==(iterator_facade const& other) const
    { return true_this().base() == other.true_this().base(); }
    
    diff_t operator>=(iterator_facade const& other) const
    { return !((*this) < other); }
    
    diff_t operator>(iterator_facade const& other) const
    { return !(((*this) < other) || ((*this) == other)); }
    
    diff_t operator<=(iterator_facade const& other) const
    { return !((*this) > other); }

    diff_t operator!=(iterator_facade const& other) const
    { return !((*this) == other); }
};

template <typename iter_t, typename diff_t>
iter_t operator+(diff_t n, iterator_facade<iter_t, diff_t> const& iter)
{
    return iter + n;
}

template <typename iter_t, typename diff_t>
iter_t operator-(diff_t n, iterator_facade<iter_t, diff_t> const& iter)
{
    return iter - n;
}

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

template <typename T, int N>
struct component;

template <typename T1, typename T2>
struct component<pair<T1, T2>, 1>
{
    using value_type = T1;
    using reference = T1&;
    using const_reference = T1 const&;
    using pointer = T1*;
    using const_pointer = T1 const*;
};

template <typename T1, typename T2>
struct component<pair<T1 const, T2>, 1>
{
    using value_type = T1;
    using reference = T1 const&;
    using const_reference = T1 const&;
    using pointer = T1 const*;
    using const_pointer = T1 const*;
};

template <typename T1, typename T2>
struct component<pair<T1, T2>, 2> : component<pair<T2, T1>, 1>
{};

template <typename underlying_iter_t, int N>
class pair_iterator
: public iterator_facade<pair_iterator<underlying_iter_t, N>, ptrdiff_t>
{
    static const bool IS_MUTABLE = is_mutable_iterator<underlying_iter_t>::value;
    underlying_iter_t iter{};
    
    using traits_t = iterator_traits<underlying_iter_t>;
    using component_t = component<typename traits_t::value_type, N>;
    
    using ref_t = typename component_t::reference;
    using cref_t = typename component_t::const_reference;
    
    using ptr_t = typename component_t::pointer;
    using cptr_t = typename component_t::const_pointer;
    
    template <typename pair_t>
    ref_t ref(pair_t& pair, integral_constant<int, 1>) const
    { return pair.first; }
    
    template <typename pair_t>
    ref_t ref(pair_t& pair, integral_constant<int, 2>) const
    { return pair.second; }
    
    template <typename pair_t>
    cref_t ref(pair_t const& pair, integral_constant<int, 1>) const
    { return pair.first; }
    
    template <typename pair_t>
    cref_t ref(pair_t const& pair, integral_constant<int, 2>) const
    { return pair.second; }
    
public:

    using value_type = typename component_t::value_type;
    using difference_type = typename traits_t::difference_type;
    using pointer = typename conditional<IS_MUTABLE, ptr_t, cptr_t>::type;
    using reference = typename conditional<IS_MUTABLE, ref_t, cref_t>::type;
    using iterator_category = typename traits_t::iterator_category;
    
    pair_iterator(underlying_iter_t it) : iter(it) {}
    
    underlying_iter_t& base()   
    { return iter; }
    
    underlying_iter_t const& base() const
    { return iter; }
    
    reference operator*() const
    { return ref(*iter, integral_constant<int, N>()); }
    
    underlying_iter_t operator->()
    { return iter; }
};

template <int N, typename iterator_t>
pair_iterator<iterator_t, N> select(iterator_t iter)
{
    return pair_iterator<iterator_t, N>(iter);
}

int main()
{
    using map_t = map<int, double>;
    
    map_t m {{1,1.1},{2,2.2},{3,3.3}};
    
    for_each(select<1>(m.begin()), select<1>(m.end()), [](auto& elem){cout << elem << '\t';});
    cout << '\n';
    for_each(select<2>(m.begin()), select<2>(m.end()), [](auto& elem){elem *= 2;});
    
    map_t const& cm = m;
    
    for_each(select<1>(cm.begin()), select<1>(cm.end()), [](auto& elem){cout << elem << '\t';});
    
    // ERROR: the second component of a const pair is a const too
    //for_each(select<2>(cm.begin()), select<2>(cm.end()), [](auto& elem){elem *= 2;});
    
    return 0;   
}

/*
1	2	3	
1	2	3
*/

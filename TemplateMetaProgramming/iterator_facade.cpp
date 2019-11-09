#include <iostream>
#include <cstddef>
#include <algorithm>
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

template <typename underlying_iter_t>
class iter_wrapper
: public iterator_facade<iter_wrapper<underlying_iter_t>, typename iterator_traits<underlying_iter_t>::difference_type>
, std::iterator_traits<int const*>
{
    underlying_iter_t iter{};
    
public:

    using value_type = typename iterator_traits<underlying_iter_t>::value_type;
    using difference_type = typename iterator_traits<underlying_iter_t>::difference_type;
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = value_type const*;
    using const_reference = value_type const&;
    using iterator_category = typename iterator_traits<underlying_iter_t>::iterator_category;
    
    iter_wrapper(underlying_iter_t it) : iter(it) {}
    
    underlying_iter_t& base()   
    { return iter; }
    
    underlying_iter_t const& base() const
    { return iter; }
    
    reference operator*()
    { return *iter; }
    
    const_reference operator*() const
    { return *iter; }
    
    underlying_iter_t operator->()
    { return iter; }
};

int main()
{
    int arr[] = {1,2,3,4,5};
    iter_wrapper arrIterBeg{arr};
    iter_wrapper arrIterEnd{arr+5};
    
    for_each(arrIterBeg, arrIterEnd, [](auto const& elem){cout << elem << '\t';});
    cout << '\n';
    
    return 0;   
}


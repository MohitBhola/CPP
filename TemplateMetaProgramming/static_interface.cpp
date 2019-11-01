#include <iostream>
using namespace std;

template <typename T>
struct instance_of
{};

template <typename static_type, typename aux_t = void>
class static_interface
{
public:

    static_type clone() const
    {
        return true_this();
    }
    
protected:

    static_interface() = default;
    ~static_interface() = default;
    
    static_type& true_this() 
    {
        return static_cast<static_type&>(*this);
    }
    
    static_type const& true_this() const
    {
        return static_cast<static_type const&>(*this);
    }
};

template <typename static_type, typename value_t>
class IDerived : public static_interface<static_type, value_t>
{
protected:

    ~IDerived() noexcept = default;
    
public:

    void foo() const
    {
        this->true_this().fooImpl(instance_of<value_t>());
    }
    
    void bar() const
    {
        this->true_this().barImpl(instance_of<value_t>());
    }
};

class DerivedImpl 
: public IDerived<DerivedImpl, int>
, public IDerived<DerivedImpl, float>
, public IDerived<DerivedImpl, double>
{

public:

    void fooImpl(instance_of<int>) const
    {
        cout << "DerivedImpl::foo(int)" << '\n';
    }
    void fooImpl(instance_of<float>) const
    {
        cout << "DerivedImpl::foo(float)" << '\n';
    }
    void fooImpl(instance_of<double>) const
    {
        cout << "DerivedImpl::foo(double)" << '\n';
    }
    
    void barImpl(instance_of<int>) const
    {
        cout << "DerivedImpl::bar(int)" << '\n';
    }
    void barImpl(instance_of<float>) const
    {
        cout << "DerivedImpl::bar(float)" << '\n';
    }
    void barImpl(instance_of<double>) const
    {
        cout << "DerivedImpl::bar(double)" << '\n';
    }
};

template <typename value_t, typename static_type>
void func(IDerived<static_type, value_t> const& iref)
{
    iref.foo();
    iref.bar();
}

int main()
{
    DerivedImpl impl{};
    func<int>(impl);
    func<float>(impl);
    func<double>(impl);

    return 0;
}

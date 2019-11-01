#include <iostream>
using namespace std;

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

template <typename static_type>
class IDerived : public static_interface<static_type>
{
protected:

    ~IDerived() noexcept = default;
    
public:

    void foo() const
    {
        this->true_this().foo();
    }
    
    void bar() const
    {
        this->true_this().bar();
    }
};

class DerivedImpl : public IDerived<DerivedImpl>
{

public:

    void foo() const
    {
        cout << "DerivedImpl::foo()" << '\n';
    }
    
    void bar() const
    {
        cout << "DerivedImpl::bar()" << '\n';
    }
};

template <typename T>
void func(IDerived<T> const& iref)
{
    iref.foo();
    iref.bar();
}

int main()
{

    DerivedImpl impl{};
    func(impl);

    return 0;
}

#include <iostream>
using namespace std;

template <typename derived_t>
struct summable
{
    
protected:

    // only classes that derive from summable<derived_t> shall be 
    // able to build a summable<derived_t>
    ~summable() noexcept = default;
    
public:
    
    // derived_t (and only derived_t) inherits publicly from summable<derived_t>
    // we can thus be sure that the runtime object would be of type derived_t
    // so, a summable<derived_> can cast to derived_t 
    derived_t& true_this()
    {
        return static_cast<derived_t&>(*this);
    }
    
    derived_t const& true_this() const
    {
        return static_cast<derived_t const&>(*this);
    }
    
    // when summable is instantiated with some derived_t, *inject* a 
    // concrete operator+ function (whose return type and parameters are then known) 
    // in the namespace of the class template summable
    friend derived_t operator+(summable const& a, summable const& b)
    {
        // the friend function has full access to summable members
        derived_t result(a.true_this());
        result += b.true_this();
        return result;
    }
};

struct Foo : summable<Foo>
{
    int ival{};

    explicit Foo(int i) : ival(i) {}
    
    Foo& operator+=(Foo const& other)
    {
        ival += other.ival;
        return *this;
    }
    
    friend ostream& operator<<(ostream& os, Foo const& foo)
    {
        cout << foo.ival;
        return os;
    }
};

int main()
{
    Foo foo1{42};
    Foo foo2{42};
    
    // there is no operator+ available in the class Foo
    // BUT, an operator+ function is available at the namespace level
    // of the class template summable which takes two summable<Foo>'s via const reference
    // and returns a Foo
    // so, the compiler calls that function
    cout << foo1 + foo2 << '\n';
    
    return 0;
}

/*
84
*/

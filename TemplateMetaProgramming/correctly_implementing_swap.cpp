#include <vector>
#include <iostream>

namespace ns_bar
{

// to optimize swap, provide a public member function swap that...
class Bar
{
    std::vector<int> ivec_{};

public:

    Bar(int n, int v) : ivec_(n, v) {}

    // 1. ...provides a using declaration for std::swap
    // 2. invokes *unqualified swap* on data members...
    void swap(Bar& other)
    {
        std::cout << "Bar::swap()" << '\n';

        using std::swap;
        swap(ivec_, other.ivec_);
    }
};

void swap(Bar& a, Bar& b)
{
    std::cout << "ns_bar::swap(Bar&, Bar&)" << '\t';
    a.swap(b);
}

}

// [SUBTLE]
// It isn't strictly necessary (but see more comments in the code that follows)
// to provide a full specialization of std::swap for the types under consideration
// *if* we have already provided an overloaded swap in the namespace in which those
// types have been defined, thanks to ADL (Koenig Lookup).

// BUT, client code may still explicitly qualify calls to swap with std:: prefix (by mistake or ignorance)
// and would thus hit the general swap in the std namespace, thus bypassing the overloaded swap in the
// namespace in which those types have been defined. If such types support move operations, the general swap in
// the std namespace would probably still do something optimal than copying, but the fact remains that
// the overloaded swap in the namespace in which those types have been defined was the right candidate to invoke,
// but the client code missed out on that.

// To provide a safety net for such a fiasco, we also provide a full specialization of std::swap for the
// types under consideration, which re-routes to member swap!

// To summarize, thou shall: 
// 1. Provide an overloaded swap in the namespace in which those types have been defined (that routes to the member swap).
// AND 
// 2. Fully specialize std::swap for those types (that routes to the member swap).

namespace std
{
    // ...and, most importantly, provide a full specialization of
    // std::swap for the type(s) under consideration
    template<>
    void swap<ns_bar::Bar>(ns_bar::Bar& bar1, ns_bar::Bar& bar2)
    {
        std::cout << "std::swap<ns_bar::Bar>(ns_bar::Bar&, ns_bar::Bar&)" << '\t';
        bar1.swap(bar2);
    }
}

namespace ns_foo
{

template<
    typename T>
class Foo
{
    T t_{};

public:

    Foo(T t) : t_(t) {}

    void swap(Foo<T>& other)
    {
        // [SUBTLE]
        // We have two options here.

        // First, we explicitly invoke std::swap, and hope that a full specialization is in place that
        // re-routes to the member swap for types that have defined a (probably more efficient) member swap.
        // If such a full specialization isn't in place, we will miss out on the (probably more efficient)
        // member swap for such types and end up invoking the (probably more expensive) classic three step std::swap(...).

        // Second, we provide a using declaration (using std::swap;), and invoke an *unqualified* call to swap(...).
        // This has the benefit of resolving to the swap overload (if defined) in the namespace of the types
        // under consideration. Of course, such swap overloads simply re-route to the member swap.
        // However, if such an overload isn't available, but a full specialization of std::swap is in place
        // that re-routes to the member swap, we're still going good. Only in the absence of *both* a swap
        // overload in the namespace of the types under considerationand a full specialization of std::swap,
        // we end up invoking the (probably more expensive) classic three step std::swap(...).

        // Note that for the second option, if we do not provide the using declaration, the name lookup for the
        // unqualified swap(...) call fails as the only candidate available is Foo<T>::swap(Foo<T>&) which doesn't
        // accepts the arguments to the call. A using declaration is *necessary* in this case to escape the class
        // scope while doing name resolution for the unqualified swap(...) call.

        using std::swap;
        swap(t_, other.t_);
    }
};

}

int main(int argc, char **argv)
{
    ns_bar::Bar bar1{42, 42};
    ns_bar::Bar bar2{43, 43};

    ns_foo::Foo<ns_bar::Bar> iFoo1{bar1};
    ns_foo::Foo<ns_bar::Bar> iFoo2{bar2};

    iFoo1.swap(iFoo2);

    // this call will resolve to the overloaded swap in the namespace ns_bar
    using namespace std;
    using namespace ns_bar;
    swap(bar1, bar2);

    return 0;
}

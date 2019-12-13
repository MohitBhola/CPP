#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <boost/operators.hpp>

template <typename...>
using VoidT = void;

template <typename T, typename = VoidT<>>
struct EnableIfIndirection : std::false_type
{};

template <typename T>
struct EnableIfIndirection<T, VoidT<decltype(std::declval<T>().operator->())>> : std::true_type
{};

template 
<
    typename Resource,
    typename mutex_t = std::recursive_mutex,
    typename lock_t  = std::unique_lock<mutex_t>
>
class thread_safe
{
    // a thread_safe *owns* its underlying
    // as long as a thread_safe object is accessible, its underlying is accessible too
    // ideally, a thread_safe object would wrap around a shared, global/namespace level object
    // it could also wrap a data member of a class/struct that could be accessed in different threads
    
    std::unique_ptr<Resource> ptr; // the underlying
    std::unique_ptr<mutex_t> mtx;  // the protection

    // The Surrogate
    // what's this?
    // it is a class template whose instantiation(s) provide proxy objects (on the fly) 
    // every time a thread safe object is accessed with an intent to access the underlying
    // it locks (explicitly) the associated mutex in its ctor and unlocks (implicitly) the same in its destructor 
    // once constructed, it allows for indirection to the underlying
    // it also provides for an implicit conversion to the underlying
    // 
    // this class template is specialized for underlying types that are themselves wrappers
    // for example, shared_ptr<>, unique_ptr<> etc
    // why?
    // to provide similar indirection semantics as for normal types
    // the aim is to let clients write (thread safe) code like this:
    // 
    // struct Foo {...};
    // thread_safe<Foo> tsFoo{};
    // thread_safe<shared_ptr<Foo>> tsFooSp{std::make_shared<Foo>()};
    // 
    // tsFoo->DoSomething();   // OK
    // tsFooSp->DoSomething(); // Also OK
    // 
    // note how the usage is uniform, *irrespective* of the *nature* of the underlying
    // this is made possible via a specialization for this class template for wrappers
    
    // primary template
    template
    <
        typename ResourceT,
        typename requested_lock_t,
        bool = EnableIfIndirection<ResourceT>::value
    >
    class proxy
        : private boost::operators<proxy<ResourceT, requested_lock_t, EnableIfIndirection<ResourceT>::value>>
    {
        ResourceT * const ptr{nullptr};
        requested_lock_t lock {};

    public:

        proxy(ResourceT * const p, mutex_t& mtx)
        : ptr(p), lock(mtx) {}

        proxy(proxy&& rhs)
        : ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}

        proxy& operator=(proxy const& rhs)
        {
            *ptr = *rhs.ptr;
             return *this;
        }

        ResourceT* operator->() { return ptr; }
        ResourceT const* operator->() const { return ptr; }

        operator ResourceT&() { return *ptr; }
        operator ResourceT const&() const { return *ptr; }

        decltype(auto) operator[](std::size_t arg) {return (*ptr)[arg];}

        ////////////////////////////////
        // for boost::totally_ordered<T>
        ////////////////////////////////
        friend auto operator<(proxy const& lhs, proxy const& rhs)
        {
            return *lhs.ptr < *rhs.ptr;
        }

        friend auto operator==(proxy const& lhs, proxy const& rhs)
        {
            return *lhs.ptr == *rhs.ptr;
        }

        ///////////////////////////////////
        // for boost::integer_arithmetic<T>
        ///////////////////////////////////
        proxy& operator+=(proxy const& rhs)
        {
            *ptr += *rhs.ptr;
            return *this;
        } 

        proxy& operator-=(proxy const& rhs)
        {
            *ptr -= *rhs.ptr;
            return *this;
        }

        proxy& operator*=(proxy const& rhs)
        {
            *ptr *= *rhs.ptr;
            return *this;
        }

        proxy& operator/=(proxy const& rhs)
        {
            *ptr /= *rhs.ptr;
            return *this;
        }

        proxy& operator%=(proxy const& rhs)
        {
            *ptr %= *rhs.ptr;
            return *this;
        }

        ////////////////////////
        // for boost::bitwise<T>
        ////////////////////////
        proxy& operator&=(proxy const& rhs)
        {
            *ptr &= *rhs.ptr;
            return *this;
        }

        proxy& operator|=(proxy const& rhs)
        {
            *ptr |= *rhs.ptr;
            return *this;
        }

        ///////////////////////////////
        // for boost::unit_steppable<T>
        ///////////////////////////////
        proxy& operator++()
        {
            ++(*ptr);
            return *this;
         }

         proxy& operator--()
         {
             --(*ptr);
             return *this;
         }
    };
    
    // the surrogate specialized for types that provide indirection
    template
    <
        typename ResourceT,
        typename requested_lock_t
    >
    class proxy<ResourceT, requested_lock_t, true>
        : private boost::operators<proxy<ResourceT, requested_lock_t, true>>
    {
        ResourceT * const ptr{nullptr};
        requested_lock_t lock {};

    public:

        proxy(ResourceT * const p, mutex_t& mtx)
        : ptr(p), lock(mtx) {}

        proxy(proxy&& rhs)
        : ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}

        proxy& operator=(proxy const& rhs)
        {
            *ptr = *rhs.ptr;
             return *this;
        }

        auto operator->() { return ptr->operator->(); }
        auto const operator->() const { return ptr->operator->(); }

        operator ResourceT&() { return ptr->operator*(); }
        operator ResourceT const&() const { return ptr->operator*(); }      
    };


public:

    template <typename... Args>
    thread_safe(Args&&... args)
    : ptr(std::make_unique<Resource>(std::forward<Args>(args)...)), mtx(std::make_unique<mutex_t>()) {}

    void lock() {mtx->lock();}
    void unlock() {mtx->unlock();}
    auto try_lock() {return mtx->try_lock();}

    auto operator->() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
};

struct Foo
{
    Foo() = default;

    void doSomething1() 
    {
        std::cout << "Foo::doSomething1()\n";
    }
    
    void doSomething2() 
    {
        std::cout << "Foo::doSomething2()\n";
    }
    
    void doSomething3() 
    {
        std::cout << "Foo::doSomething3()\n";
    }
   
    int i{42};
    float f{42.0};
    char c{'a'};
};


// works with fundamental types
thread_safe<int> safeInt(42);

// works with user defined types
thread_safe<Foo> safeFoo{};

// works with library types
thread_safe<std::map<int, int>> safeMap{};
thread_safe<std::map<int, int>> safeMap_copy{};

thread_safe<std::string> safeStr1{"abc"};
thread_safe<std::string> safeStr2{"xyz"};

// works with library *wrapper* types (that provide indirection) too!
thread_safe<std::shared_ptr<Foo>> safeSP{std::make_shared<Foo>()};

void f1()
{
    // thread_safe<int> supports increment-assignment operator
    // since the underlying int supports the same
    *safeInt += 42;

    // thread safe singular operation
    safeFoo->doSomething1();

    // thread safe transactional semantics
    // thread_safe implements the BasicLockable interface

    safeFoo.lock();
    safeFoo->doSomething2();
    safeFoo->doSomething3();
    safeFoo.unlock();
}

void f2()
{
    // thread_safe<int> supports increment-assignment operator
    // since the underlying int supports the same
    *safeInt += 42;

    // thread safe singular operation
    safeFoo->doSomething2();

    // thread safe transactional semantics
    // thread_safe implements the BasicLockable interface
    // lock is released at the end of the comma operator
    safeFoo.lock();
    safeFoo->doSomething2();
    safeFoo->doSomething3();
    safeFoo.unlock();
}

// either f3 or f4 populates safeMap_copy
// depending on which thread successfully acquires locks on both safeMap and safeMap_copy first

// thread_safe<std::map> allows for assignment
// since the underlying std::map supports the same

// [SUBTLE]
// we need to acquire a lock on both underlying shared resources (std::map)
// but different threads may try to acquire those locks
// in different orders which is a classic recipe for a deadlock
// for example, note that f3() and f4() below acquire locks in different orders
// we need to acquire these locks atomically
// thankfully, std::lock() allows just that!

void f3()
{
    {
        std::lock(safeMap, safeMap_copy); // transactional semantics

        if (safeMap_copy->empty())
        {
            *safeMap_copy = *safeMap;
        }

        safeMap.unlock();
        safeMap_copy.unlock();
    }
}

void f4()
{
    {
        std::lock(safeMap_copy, safeMap); // transactional semantics; note different order of lock acquisition than in f3()

        if (safeMap_copy->empty())
        {
            *safeMap_copy = *safeMap;
        }

        safeMap.unlock();
        safeMap_copy.unlock();
    }
}

int main()
{
    std::thread t1(f1);
    std::thread t2(f2);

    t1.join();
    t2.join();

    // thread_safe<int> allows access to the underlying shared resource (int) via dereferencing
    // this is guaranteed to print 126 since increments happen
    // atomically in threads t1 and t2
    std::cout << *safeInt << '\n';

    // thread_safe<Foo> allows for transparent indirection for member access
    std::cout << safeFoo->c << '\n';

    // thread_safe<std::map> allows for subscripting as the underlying
    // shared resource (a std::map) supports the same
    (*safeMap)[1] = 1;
    (*safeMap)[2] = 2;

    std::cout << (*safeMap)[1] << '\n';
    std::cout << (*safeMap)[2] << '\n';

    std::thread t3(f3);
    std::thread t4(f4);

    t3.join();
    t4.join();

    // safeMap_copy got populated in a thread safe manner
    // in either thread t3 or thread t4
    std::cout << (*safeMap_copy)[1] << '\n';
    std::cout << (*safeMap_copy)[2] << '\n';

    // thread_safe<std::string> allows for comparisons
    // as the underlying shared resource (std::string) supports the same
    // std::lock used here for transactional semantics
    {
        std::lock(safeStr1, safeStr2);
        std::cout << std::boolalpha << (*safeStr1 > *safeStr2) << '\n';
        std::cout << std::boolalpha << (*safeStr1 != *safeStr2) << '\n';
    }
    
    safeSP->doSomething1();
    
    return 0;
}

/*
Foo::doSomething1()
Foo::doSomething2()
Foo::doSomething3()
Foo::doSomething2()
Foo::doSomething2()
Foo::doSomething3()
126
a
1
2
1
2
false
true
Foo::doSomething1()
*/

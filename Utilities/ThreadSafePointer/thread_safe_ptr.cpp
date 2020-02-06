#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <string>
#include <thread>

// a manual replacement for std::void_t (available since C++17)
template<
    typename...>
using VoidT = void;

template<
    typename T, typename = VoidT<>>
struct DisableIfIndirection : std::false_type
{};

template<
    typename T>
struct DisableIfIndirection<T, VoidT<decltype(std::declval<T>().operator->())>> : std::true_type
{};

// the primary template
// later specialized for underlying types that are wrappers themselves, that is, types that implement operator->()
// please read more comments elsewhere
template<
    typename Resource,
    typename mutex_t = std::recursive_mutex,
    typename lock_t  = std::unique_lock<mutex_t>,
    bool = DisableIfIndirection<Resource>::value>
class thread_safe_ptr
{
    /*
    
     A thread_safe_ptr object *owns* an underlying object. There could be 2 types of underlyings:
    
     1. A normal, regular type that isn't providing wrapper semantics. That is, a type that doesn't overloads the indirection operator. This includes
        both fundamental types and most user defined types.
     
        For such types, a thread_safe_ptr could be created as follows:
        
        a. Pass (to a thread_safe_ptr ctor) no arguments / all arguments necessary to invoke the default / user defined ctor of the underlying.
        b. Pass (to a thread_safe_ptr ctor) an lvalue / rvalue reference to the underlying. As usual, lvalues would be copied in, and rvalues would be moved in.
        c. Invoke thread_safe_ptr's copy construction / assignment.
        
        The thread_safe_ptr object thus constructed shall aggregate a shared_ptr to the underlying. As such, each such thread_safe_ptr object shall *always* 
        have an underlying, and clients need not check them for NULL prior to dereferencing. 
        
        As aforementioned in # c, it is possible to copy a thread_safe_ptr object. Each such copy shall contain its own shared_ptr to the *same* underlying, 
        and would provide thread-safe access to that *same* underlying.
        
        It isn't possible to move a thread_safe_ptr object, though. Why? If move semantics are supported, the moved-from thread_safe_ptr object shall 
        lose its underlying, and thus couldn't be dereferenced. This would necessitate the client code to first check for NULL prior to dereferencing. 
    
     2. A *wrapper*, such as a shared_ptr<T>, or an IRef (in the DVA world). That is, something that has an underlying of its own, 
        and could be detected at compile time (SFINAE) via the availability of an overloaded indirection operator (operator->()).
        
        As an aside, note that as per classic C++ language rules, the result of an indirection should either result in a raw pointer, 
        or should result in an object of a class that itself overloads the indirection operator. The process continues until the compiler 
        arrives at a raw pointer. If not, the compile emits an error.
        
        For such types, a thread_safe_ptr could be created as follows:
        
        a. Pass (to the thread_safe_ptr ctor) no arguments / all arguments necessary to invoke the default / user defined ctor of the underlying wrapper.
        b. Pass (to the thread_safe_ptr ctor) an rvalue reference to the underlying, which would be moved in. Note that lvalue references are disallowed here. 
           More on this below.
        c. Invoke thread_safe_ptr's copy/move construction or copy/move assignment.
        
        For such *wrapper* underlyings, the thread_safe_ptr object shall *directly* aggregate them. Note that we would like to access both the 
        underlying wrapper, and the underlying wrapper's underlying. For example, if the underlying wrapper is shared_ptr<Foo>, we would like access the 
        public interface of both shared_ptr<Foo> (to invoke, for example, its reset() API), and the underlying Foo as well! 
        The former requirement implies that clients of such thread_safe_ptr objects should be prepared to check the wrapper underlying to be NULL prior to accessing the real underlying. 
        This also implies that such thread_safe_ptr objects are both copyable and moveable.
        
        [SUBTLE]
        
        Why can't we create a thread_safe_ptr object via an lvalue reference to a wrapper underlying? Because the ownership group of the real underlying needs 
        to be a closed one. If there remains an lvalue reference to a wrapper underlying after having constructed one or more thread_safe_ptr's from it, the access to 
        the real underlying from that lvalue reference shall *NOT* be thread safe. Hence it is forbidden.  
    */
        
    std::shared_ptr<Resource> ptr{}; // the underlying
    std::shared_ptr<mutex_t> mtx{};  // the protection
    
    // The Surrogate
    // What's this?
    //
    // To access the underlying shared resource, clients *always* need to invoke the Lock() API on the thread_safe_ptr
    // This invocation returns a proxy object which provides for the thread safe access to the underlying. 
    // The associated mutex of the shared resource gets locked (explicitly) in the proxy's ctor, and the same gets released (implicitly) in its dtor.
    // The proxy objects are thus used in an RAII based manner in client code.
    // It also provides for an implicit conversion to the underlying.
    
    template<
        typename ResourceT,
        typename requested_lock_t>
    class proxy
    {
        ResourceT* pUnderlying{nullptr};
        requested_lock_t lock{};

    public:
        
        // by default, the underlying isn't available and thus the lock on the underlying isn't acquired
        proxy() = default;
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceT* const p, mutex_t& mtx)
        : pUnderlying(p), lock(mtx) /* HERE */ {}

        proxy(proxy&& rhs)
        : pUnderlying(std::move(rhs.pUnderlying)), lock(std::move(rhs.lock)) {}
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;

        // overload the indirection operator to reach out to the underlying 
        ResourceT* operator->() { return pUnderlying; }
        ResourceT const* operator->() const { return pUnderlying; }

        // overload the dereference operator to reach out to the underlying        
        ResourceT& operator*() { return *pUnderlying; }
        ResourceT const& operator*() const { return *pUnderlying; }
    };

public:

    // default constructing a thread_safe_ptr assumes that it is possible to default construct the underlying
    thread_safe_ptr()
    : ptr(std::make_unique<Resource>()), mtx(std::make_shared<mutex_t>()) {}

    // universal ctor
    // intended to construct the underlying by invoking a viable ctor of the underlying using the parameters of this universal ctor
    // such viable ctors include normal ctors taking multiple arguments as well as copy / move ctors
    //
    // [SUBTLE]
    // this ctor would shadow the normal copy ctor of thread_safe_ptr while trying to create a copy of a thread_safe_ptr
    // thus disabled for such scenarios
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!std::is_same<std::decay_t<T>, thread_safe_ptr>::value>>
    thread_safe_ptr(T&& t, Args&&... args)
    : ptr(std::make_shared<Resource>(std::forward<T>(t), std::forward<Args>(args)...)), mtx(std::make_shared<mutex_t>()) {}
    
    // copy construction / assignment
    thread_safe_ptr(thread_safe_ptr const& source) = default;
    thread_safe_ptr& operator=(thread_safe_ptr const&) = default;

    // *don't* provide move semantics
    thread_safe_ptr(thread_safe_ptr const&&) = delete;
    thread_safe_ptr& operator=(thread_safe_ptr&&) = delete;
    
    // implement the *BasicLockable* interface
    // enables thread_safe_ptr objects to be used in the std::lock(...) API
    // helps to acquire a lock on multiple thread_safe_ptr objects in different threads without risking a deadlock
    void lock() { mtx->lock(); }
    bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    // ALL and ANY access to the underlying is via the Lock() API
    // helps to eliminate the class of bugs arising out of locking the wrong mutex or forgetting to lock a mutex while trying to access the shared underlying
    auto Lock() 
    {
        return ptr ? proxy<Resource, lock_t>(ptr.get(), *mtx) : proxy<Resource, lock_t>(); 
    }
    
    auto Lock() const
    {
        return ptr ? proxy<Resource const, lock_t>(ptr.get(), *mtx) : proxy<Resource const, lock_t>(); 
    }
    
};

// thread_safe_ptr specialized for *wrapper* underlyings
// note that we would like to access the public interface of both the wrapper underlying, as well as the wrapper underlying's underlying
template<
    typename Resource,
    typename mutex_t,
    typename lock_t>
class thread_safe_ptr<Resource, mutex_t, lock_t, true>
{    
    Resource res;                  // the underlying (note direct aggregation!)
    std::shared_ptr<mutex_t> mtx;  // the protection
        
    template<
        typename ResourceT,
        typename requested_lock_t>
    class proxy
    {
        ResourceT * const pUnderlying{nullptr};
        requested_lock_t lock {};

    public:
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceT* const p, mutex_t& mtx)
        : pUnderlying(p), lock(mtx) /* HERE */ {}

        proxy(proxy&& rhs)
        : pUnderlying(std::move(rhs.pUnderlying)), lock(std::move(rhs.lock)) {}
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;
        
        // [SUBTLE]
        // *wrapper* underlyings may get reset
        // thus, we need to provide a way for the clients to check for the same (via an implicit conversion to bool) prior to dereferencing it
        operator bool() const
        {
            return static_cast<bool>(pUnderlying ? *pUnderlying : 0);
        }

        // (special) overloaded indirection to the wrapper underlying's underlying
        // this let's the client code to access the public interface of the wrapper underlying's underlying via a consistent syntax 
        auto* operator->() { return pUnderlying->operator->(); }
        auto const* operator->() const { return pUnderlying->operator->(); }

        // overload the dereference operator to reach out to the *wrapper's underlying*        
        // this lets the clients to access the public interface of the wrapper underlying itself
        // for example, to access the reset() API of the underlying shared_ptr<Foo>
        ResourceT& operator*() { return *pUnderlying; }
        ResourceT const& operator*() const { return *pUnderlying; }
    };
        
public:

    // universal ctor
    // intended to construct the underlying by invoking a viable ctor of the underlying using the parameters of this universal ctor
    // such viable ctors include normal ctors taking multiple arguments as well as copy / move ctors
    //
    // [SUBTLE]
    // this ctor would shadow the normal copy ctor of thread_safe_ptr while trying to create a copy of a thread_safe_ptr
    // thus disabled for such scenarios
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!(std::is_lvalue_reference<T>::value && std::is_same<Resource, std::decay_t<T>>::value)>,
        typename = std::enable_if_t<!std::is_same<std::decay_t<T>, thread_safe_ptr>::value>>
    thread_safe_ptr(T&& t, Args&&... args)
    : res(std::forward<T>(t), std::forward<Args>(args)...), mtx(std::make_shared<mutex_t>()) {}
    
    // provide for copy construction / assignment
    thread_safe_ptr(thread_safe_ptr const& source) = default;
    thread_safe_ptr& operator=(thread_safe_ptr const&) = default;
    
    // provide for move construction / assignment
    thread_safe_ptr(thread_safe_ptr const&&) = delete;
    thread_safe_ptr& operator=(thread_safe_ptr&&) = delete;
    
    // implement the *BasicLockable* interface
    // enables thread_safe_ptr objects to be used in the std::lock(...) API
    // helps to acquire a lock on multiple thread_safe_ptr objects in different threads without risking a deadlock
    void lock() { mtx->lock(); }
    bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }
    
    // ALL and ANY access to the underlying is via the Lock() API
    // helps to eliminate the class of bugs arising out of locking the wrong mutex or forgetting to lock a mutex while trying to access the shared underlying
    auto Lock() 
    {
        return proxy<Resource, lock_t>(std::addressof(res), *mtx);
    }
    
    auto const Lock() const 
    {
        return proxy<Resource const, lock_t>(std::addressof(res), *mtx);
    }
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
thread_safe_ptr<int> safeInt(42);

// works with user defined types
thread_safe_ptr<Foo> safeFoo{};

// works with library types
thread_safe_ptr<std::map<int, int>> safeMap{};
thread_safe_ptr<std::map<int, int>> safeMap_copy{};

thread_safe_ptr<std::string> safeStr1{"abc"};
thread_safe_ptr<std::string> safeStr2{"xyz"};

// works with *wrapper* types (that provide indirection) too!
thread_safe_ptr<std::shared_ptr<Foo>> safeSP{std::make_shared<Foo>()};

void f1()
{
    {
        // in order to do anything with the underlying, clients first need to invoke the Lock() API on the synchronized ptr
        // otherwise, the code won't compile
        // this would prevent the class of bugs arising out of having locked a wrong mutex or forgotten to lock a mutex at all
        // while trying to access the shared resource
        auto intRef = safeInt.Lock();
        
        // once we have obtained the proxy to an underlying, clients need to dereference it to do a thread safe access to that underlying
        *intRef += 42;
    } // the proxy goes out of scope here and releases the lock on the underlying
      // this covers both normal returns and exceptions as well

    // performing a thread safe singular operation
    // indirection is *natural*; just use operator-> on the proxy returned by the thread_safe_ptr, and it seamlessly redirects to the underlying
    {
        auto fooRef = safeFoo.Lock();
        fooRef->doSomething1();
    }
    
    // performing a thread safe transaction
    {
        auto fooRef = safeFoo.Lock();
        fooRef->doSomething2();
        fooRef->doSomething3();
    }
}

void f2()
{
    {
        // accessing the safeInt in a thread safe manner along with such an access in function f1()
        auto intRef = safeInt.Lock();
        *intRef += 42; // a thread safe increment
    }

    // a thread safe singular operation, as in function f1()
    {
        auto fooRef = safeFoo.Lock();
        fooRef->doSomething2();
    }

    // a thread safe transaction, as in function f1()
    {
        auto fooRef = safeFoo.Lock();
        fooRef->doSomething2();
        fooRef->doSomething3();
    }
}

// the functions f3() and f4() below are made to run on different threads, and each tries to copy assign to safeMap_copy from safeMap
// either f3() or f4() does the copy assignment, depending on which of them successfully acquires a lock on *both* safeMap and safeMap_copy first

// [SUBTLE]
// this is an example of a transaction that involves two shared resources: safeMap_copy and safeMap
// we need to acquire a lock on *both* of them 
// but different threads may try to acquire those locks in different orders which is a classic recipe for a deadlock!
// for example, note that functions f3() and f4() below acquire those locks in different orders
// we need to acquire these locks atomically
// thankfully, std::lock() allows just that!

void f3()
{
    // avoid risking a deadlock while acquiring multiple locks by leveraging the std::lock(...) API (since C++11)
    // since a thread_safe_ptr implements the BasicLockable interface, they could be used with this API!
    // NOTE: from C++17 onwards, we should be using std::scoped_lock here, else we need to explicitly unlock the thread_safe_ptrs as below
    std::lock(safeMap, safeMap_copy);

    {
        // acquiring the proxies to access the public API of the underlying
        // this would acquire the locks again, so the mutex better be recursive
        auto mapRef = safeMap.Lock(); 
        auto mapCopyRef = safeMap_copy.Lock();
        
        if (mapCopyRef->empty())
        {
            *mapCopyRef = *mapRef;
        }   
    }
    
    // since we locked the thread_safe_ptrs explicitly, we need to unlock them explicitly too!
    // from C++17 onwards, this wouldn't be necessary by using std::scoped_lock to acquire the locks
    safeMap.unlock();
    safeMap_copy.unlock();
}

void f4()
{    
    // note different order of arguments to the std::lock(...) API than in f3()
    // this is the kind of mistake that is too easy to make by having to lock multiple mutexes manually
    // since thread_safe_ptr objects implement the BasicLockable interface, we avoid this risk altogether
    // developers just need to follow the rule of thumb that if a CriticalSection requires locking multiple thread_safe_ptr objects,
    // they should do so via std::lock(...) API; the order of argument(s) to this API doesn't matter
    std::lock(safeMap_copy, safeMap);

    {
        auto mapRef = safeMap.Lock();
        auto mapCopyRef = safeMap_copy.Lock();
        
        if (mapCopyRef->empty())
        {
            *mapCopyRef = *mapRef;
        }   
    }
    
    safeMap.unlock();
    safeMap_copy.unlock();
}

int main()
{
    std::thread t1(f1);
    std::thread t2(f2);

    t1.join();
    t2.join();

    // thread_safe_ptr<int> allows access to the underlying shared resource (int) via dereferencing
    // this is guaranteed to print 126 since increments happen in a thread safe manner in threads t1 and t2
    // NOTE: the proxy is an rvalue here and it gets destroyed (and this releases the lock on the underlying) at the end of the statement
    std::cout << *safeInt.Lock() << '\n';
    
    // thread_safe_ptr<Foo> allows for transparent indirection to access the public API of the underlying
    std::cout << safeFoo.Lock()->c << '\n';

    // populate safeMap in a thread safe manner
    {
        auto mapRef = safeMap.Lock();
        
        (*mapRef)[1] = 1;
        (*mapRef)[2] = 2;    
        
        std::cout << (*mapRef)[1] << '\n';
        std::cout << (*mapRef)[2] << '\n';
    }
    
    std::thread t3(f3);
    std::thread t4(f4);

    t3.join();
    t4.join();

    // safeMap_copy got populated in a thread safe manner
    // in either thread t3 or thread t4 (depending upon which was able to acquire a lock (atomically) on both safeMap and safeMap_copy)
    {
        auto mapCopyRef = safeMap_copy.Lock();
        
        std::cout << (*mapCopyRef)[1] << '\n';
        std::cout << (*mapCopyRef)[2] << '\n';
    }

    {
        // use std::scoped_lock from C++17 onwards...
        std::lock(safeStr1, safeStr2);
        
        auto str1Ref = safeStr1.Lock();
        auto str2Ref = safeStr2.Lock();
        
        std::cout << std::boolalpha << (*str1Ref > *str2Ref) << '\n';
        std::cout << std::boolalpha << (*str1Ref != *str2Ref) << '\n';
        
        // ...to avoid this
        safeStr1.unlock();
        safeStr2.unlock();
    }
    
    // sp is a *wrapper* underlying
    std::shared_ptr<int> sp(new int(9999));
    
    // ERROR: cannot create thread_safe_ptr objects from lvalues of wrapper underlyings.
    // Why? Because the ownership group of thread_safe_ptr objects needs to be a closed one.
    // If allowed, access to the underlying would NOT be thread safe.
    //thread_safe_ptr<std::shared_ptr<int>> ts_sp{sp}; 
    
    // can create thread_safe_ptr objects from rvalues of wrapper underlyings
    thread_safe_ptr<std::shared_ptr<int>> ts_sp1{std::move(sp)};
    
    // NOTE *double* indirection to access the wrapper underlying's underlying
    {
        auto sp1Ref = ts_sp1.Lock();
        **sp1Ref *= 2; // access the real underlying int
        std::cout << **sp1Ref << '\n';
    }
    
    {
        // can create thread_safe_ptr objects from rvalues of wrapper underlyings
        thread_safe_ptr<std::shared_ptr<Foo>> ts_sp2{std::make_shared<Foo>()}; 
    
        // indirection is *natural*; just use operator->() on the proxy, 
        // and it redirects all the way to the wrapper underlying's underlying
        auto sp2Ref = ts_sp2.Lock();
        sp2Ref->doSomething1(); // actually invoke Foo::doSomething1()
    }
    
    {
        // can create copies of thread_safe_ptrs
        thread_safe_ptr<std::shared_ptr<int>> ts_sp3{ts_sp1};
        
        auto sp3Ref = ts_sp3.Lock();
        
        // NOTE *double* dereference to access the wrapper underlying's underlying
        **sp3Ref *= 2;   
        std::cout << **sp3Ref << '\n';
        
        // NOTE *single* dereference to the wrapper underlying itself
        (*sp3Ref).reset();
        
        // proxy's implicit conversion to bool to indicate whether the wrapper underlying / wrapper underlying's underlying exists or not
        if (!sp3Ref)
        {
            std::cout << "ts_sp3's underlying is now indeed NULL\n\n";
        }
    }
     
    return 0;
}

/*
OUTPUT (-std=c++14)
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
19998
Foo::doSomething1()
39996
ts_sp3's underlying is now indeed NULL
*/

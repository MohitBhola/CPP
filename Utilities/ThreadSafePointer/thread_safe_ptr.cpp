#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <boost/operators.hpp>

// Author: @mbhola

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
        
        a. Pass (to the thread_safe_ptr ctor) no arguments / all arguments necessary to invoke the default / user defined ctor of the underlying.
        b. Pass (to the thread_safe_ptr ctor) an lvalue / rvalue reference to the underlying. As usual, lvalues would be copied in, and rvalues would be moved in.
        c. Invoke thread_safe_ptr's copy construction or copy assignment. 
        
        The thread_safe_ptr object thus constructed shall aggregate a shared_ptr to the underlying. As such, each such thread_safe_ptr object shall *always* have an 
        underlying, and clients need not check them for NULL prior to dereferencing. 
        
        As aforementioned in # c, it is possible to copy a thread_safe_ptr object. Each such copy shall contain a shared_ptr to the *same* underlying, and would provide 
        thread-safe access to that *same* underlying.
        
        It isn't possible to move a thread_safe_ptr object, though. Why? If move semantics are supported, the moved-from thread_safe_ptr object shall 
        lose its underlying, and thus couldn't be dereferenced. This would necessitate the client code to first check for NULL prior to dereferencing. 
    
     2. A *wrapper*, such as a shared_ptr<T>, or an IRef. That is, something that has an underlying of its own, and could be detected at compile time via the 
        availability of an overloaded indirection operator (operator->()).
        
        As an aside, note that as per classic C++ language rules, the result of an indirection should either result in a raw pointer, 
        or should result in an object of a class that itself overloads the indirection operator. The process continues until the compiler 
        arrives at a raw pointer. If not, the compile emits an error.
        
        For such types, a thread_safe_ptr could be created as follows:
        
        a. Pass (to the thread_safe_ptr ctor) no arguments / all arguments necessary to invoke the default / user defined ctor of the underlying wrapper.
        b. Pass (to the thread_safe_ptr ctor) an rvalue reference to the underlying, which would be moved in. Note that lvalue references are disallowed here. More on this below.
        c. Invoke thread_safe_ptr's copy/move construction or copy/move assignment.
        
        For such *wrapper* underlyings, the thread_safe_ptr object shall *directly* aggregate them. Note that we would like to access both the 
        underlying wrapper, and the underlying wrapper's underlying. For example, if the underlying wrapper is shared_ptr<Foo>, we would like access the public interface 
        of both shared_ptr<Foo> (for example, reset()), and the underlying Foo as well! The former requirement implies that clients of such thread_safe_ptr objects should be prepared 
        to check the wrapper underlying to be NULL prior to accessing the real underlying. This also implies that such thread_safe_ptr objects are both copyable and moveable.
        
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
    // it is a class template whose instantiation(s) provide proxy objects (on the fly) 
    // every time a thread safe object is accessed with an intent to access the underlying
    // it locks (explicitly) the associated mutex in its ctor and unlocks (implicitly) the same in its destructor 
    // once constructed, it allows for indirection to the underlying
    // it also provides for an implicit conversion to the underlying
    
    // primary template
    template<
        typename ResourceT,
        typename requested_lock_t>
    class proxy
    {
        ResourceT * const ptr{nullptr};
        requested_lock_t lock{};

    public:
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceT * const p, mutex_t& mtx)
        : ptr(p), lock(mtx) {}

        proxy(proxy&& rhs)
        : ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;

        // overload the indirection operator to reach out to the underlying 
        ResourceT* operator->() { return ptr; }
        ResourceT const* operator->() const { return ptr; }

        // overload the access operator to reach out to the underlying        
        ResourceT& operator*() { return *ptr; }
        ResourceT const& operator*() const { return *ptr; }
    };

public:

    // default constructing a thread_safe_ptr assumes that it is possible to default construct the underlying
    thread_safe_ptr()
    : ptr(std::make_unique<Resource>()), mtx(std::make_shared<mutex_t>()) {}

    // universal constructor
    // intended to invoke a viable ctor of the underlying 
    //
    // [SUBTLE]
    // this constructor would shadow the normal copy constructor while trying to create a copy of a thread_safe_ptr object
    // thus disabled for such scenarios
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!std::is_same<std::decay_t<T>, thread_safe_ptr>::value>>
    thread_safe_ptr(T&& t, Args&&... args)
    : ptr(std::make_unique<Resource>(std::forward<T>(t), std::forward<Args>(args)...)), mtx(std::make_shared<mutex_t>()) {}
    
    // provide copy semantics
    thread_safe_ptr(thread_safe_ptr const& source) = default;
    thread_safe_ptr& operator=(thread_safe_ptr const&) = default;

    // *don't* provide move semantics
    thread_safe_ptr(thread_safe_ptr const&&) = delete;
    thread_safe_ptr& operator=(thread_safe_ptr&&) = delete;
    
    // implement the *BasicLockable* interface
    // enables thread_safe_ptr objects to be used in the std::lock(...) API
    // to acquire a lock on them (to provide transactionable semantics) without risking a deadlock
    void lock() { mtx->lock(); }
    bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    auto operator->() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
};

// thread_safe_ptr specialized for *wrapper* underlyings
// note that we would like to access the public interface of both the wrapper underlying, as well as the wrapper underlying's underlying
template<
    typename Resource,
    typename mutex_t,
    typename lock_t>
class thread_safe_ptr<Resource, mutex_t, lock_t, true>
{    
    Resource res;                  // the underlying (note direct aggregation)
    std::shared_ptr<mutex_t> mtx;  // the protection
        
    template<
        typename ResourceT,
        typename requested_lock_t>
    class proxy
    {
        ResourceT * const ptr{nullptr};
        requested_lock_t lock {};

    public:
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceT * const p, mutex_t& mtx)
        : ptr(p), lock(mtx) {}

        proxy(proxy&& rhs)
        : ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;
        
        // [SUBTLE]
        // *wrapper* underlyings may get reset
        // thus, we need to provide a way for the clients to check for the same prior to dereferencing
        operator bool() const 
        {
            return (ptr && *ptr) ? true : false;
        }

        // (special) overloaded indirection to the wrapper underlying's underlying
        // this let's the client code to access the public interface of the wrapper underlying's underlying 
        auto* operator->() { return ptr->operator->(); }
        auto const* operator->() const { return ptr->operator->(); }

        // overload the access operator to reach out to the *wrapper* underlying        
        // this lets the clients to access the public interface of the wrapper underlying itself
        ResourceT& operator*() { return *ptr; }
        ResourceT const& operator*() const { return *ptr; }
    };
        
public:

    // universal constructor
    // intended to invoke a viable ctor of the underlying wrapper
    // disabled for construction via an lvalue reference to the underlying wrapper
    //
    // [SUBTLE]
    // this constructor would shadow the normal copy constructor while trying to create a copy of a thread_safe_ptr object
    // thus disabled for such scenarios
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!(std::is_lvalue_reference<T>::value && std::is_same<Resource, std::decay_t<T>>::value)>,
        typename = std::enable_if_t<!std::is_same<std::decay_t<T>, thread_safe_ptr>::value>>
    thread_safe_ptr(T&& t, Args&&... args)
    : res(std::forward<T>(t), std::forward<Args>(args)...), mtx(std::make_shared<mutex_t>()) {}
    
    // provide copy semantics
    thread_safe_ptr(thread_safe_ptr const& source) = default;
    thread_safe_ptr& operator=(thread_safe_ptr const&) = default;
    
    // provide move semantics
    thread_safe_ptr(thread_safe_ptr const&&) = delete;
    thread_safe_ptr& operator=(thread_safe_ptr&&) = delete;
    
    // implement the *BasicLockable* interface
    // enables thread_safe_ptr objects to be used in the std::lock(...) API
    // to acquire a lock on them (to provide transactionable semantics) without risking a deadlock
    void lock() { mtx->lock(); }
    bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    auto operator->() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    
    auto get() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const get() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
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
    // thread_safe_ptr<int> supports increment-assignment operator
    // since the underlying int supports the same
    // note double dereference to access the underlying by reference
    **safeInt += 42;

    // thread safe singular operation
    // indirection is *natural*; just use operator-> on the thread_safe_ptr, and it redirects to the underlying
    safeFoo->doSomething1();

    // thread safe transactional semantics
    // thread_safe_ptr implements the BasicLockable interface
    safeFoo.lock();
    safeFoo->doSomething2();
    safeFoo->doSomething3();
    safeFoo.unlock();
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeFoo);
        safeFoo->doSomething2();
        safeFoo->doSomething3();
    }    
    */
}

void f2()
{
    // thread_safe_ptr<int> supports increment-assignment operator
    // since the underlying int supports the same
    **safeInt += 42;

    // thread safe singular operation
    safeFoo->doSomething2();

    // thread safe transactional semantics
    // thread_safe_ptr implements the BasicLockable interface
    safeFoo.lock();
    safeFoo->doSomething2();
    safeFoo->doSomething3();
    safeFoo.unlock();
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeFoo);
        safeFoo->doSomething2();
        safeFoo->doSomething3();
    }    
    */
}

// either f3 or f4 populates safeMap_copy
// depending on which thread successfully acquires locks on *both* safeMap and safeMap_copy first

// thread_safe_ptr<std::map> allows for assignment
// since the underlying std::map supports the same

// [SUBTLE]
// we need to acquire a lock on the underlying shared resources (std::map) of both safeMap and safeMap_copy
// but different threads may try to acquire those locks in different orders which is a classic recipe for a deadlock!
// for example, note that f3() and f4() below acquire locks in different orders
// we need to acquire these locks atomically
// thankfully, std::lock() allows just that!

void f3()
{
    
    std::lock(safeMap, safeMap_copy); // transactional semantics

    if (safeMap_copy->empty())
    {
        // note double dereference to access the underlying by reference
        **safeMap_copy = **safeMap;
    }

    safeMap.unlock();
    safeMap_copy.unlock();
        
    /*
    Since C++17
    {
        std::scoped_lock lock(safeMap, safeMap_copy); // transactional semantics

        if (safeMap_copy->empty())
        {
            **safeMap_copy = **safeMap;
        }
    }
    */
}

void f4()
{    
    std::lock(safeMap_copy, safeMap); // transactional semantics; note different order of lock acquisition than in f3()

    if (safeMap_copy->empty())
    {
        **safeMap_copy = **safeMap;
    }

    safeMap.unlock();
    safeMap_copy.unlock();
   
    /*
    Since C++17
    {
        std::scoped_lock lock(safeMap_copy, safeMap); // transactional semantics

        if (safeMap_copy->empty())
        {
            **safeMap_copy = **safeMap;
        }
    }
    */
}

int main()
{
    std::thread t1(f1);
    std::thread t2(f2);

    t1.join();
    t2.join();

    // thread_safe_ptr<int> allows access to the underlying shared resource (int) via dereferencing
    // this is guaranteed to print 126 since increments happen in a thread safe manner in threads t1 and t2
    std::cout << **safeInt << '\n';

    // thread_safe_ptr<Foo> allows for transparent indirection for (public) member access
    std::cout << safeFoo->c << '\n';

    // thread_safe_ptr<std::map> allows for subscripting as the underlying
    // shared resource (a std::map) supports the same
    (**safeMap)[1] = 1;
    (**safeMap)[2] = 2;

    std::cout << (**safeMap)[1] << '\n';
    std::cout << (**safeMap)[2] << '\n';

    std::thread t3(f3);
    std::thread t4(f4);

    t3.join();
    t4.join();

    // safeMap_copy got populated in a thread safe manner
    // in either thread t3 or thread t4
    std::cout << (**safeMap_copy)[1] << '\n';
    std::cout << (**safeMap_copy)[2] << '\n';

    // thread_safe_ptr<std::string> allows for comparisons
    // as the underlying shared resource (std::string) supports the same
    // std::lock used here for transactional semantics
    {
        std::lock(safeStr1, safeStr2);
        std::cout << std::boolalpha << (**safeStr1 > **safeStr2) << '\n';
        std::cout << std::boolalpha << (**safeStr1 != **safeStr2) << '\n';
        safeStr1.unlock();
        safeStr2.unlock();
    }
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeStr1, safeStr2);
        std::cout << std::boolalpha << (*safeStr1 > *safeStr2) << '\n';
        std::cout << std::boolalpha << (*safeStr1 != *safeStr2) << '\n';
    }
    */
    
    // sp: a *wrapper* underlying
    std::shared_ptr<int> sp(new int(9999));
    
    // ERROR
    // cannot create thread_safe objects from lvalues of wrapper underlyings
    //thread_safe_ptr<std::shared_ptr<int>> ts_sp{sp}; 
    
    // OK
    // can create thread_safe_ptr objects from rvalues of wrapper underlyings
    thread_safe_ptr<std::shared_ptr<int>> ts_sp1{std::move(sp)};
    
    // NOTE *triple* indirection to access the wrapper underlying's underlying
    ***ts_sp1 *= 2;
    std::cout << ***ts_sp1 << '\n';
    
    // OK
    // can create thread_safe_ptr objects from rvalues of wrapper underlyings
    thread_safe_ptr<std::shared_ptr<Foo>> ts_sp2{std::make_shared<Foo>()}; 
    
    // indirection is *natural*; just use operator->() on the thread_safe_ptr, 
    // and it redirects all the way to the wrapper underlying's underlying
    ts_sp2->doSomething1();
    
    // OK
    // can create copies of thread_safe_ptrs
    thread_safe_ptr<std::shared_ptr<int>> ts_sp3{ts_sp1};
    
    // NOTE *triple* indirection to access the wrapper underlying's underlying
    ***ts_sp3 *= 2;   
    std::cout << ***ts_sp3 << '\n';
    
    // NOTE *double* indirection to the wrapper underlying itself
    (**ts_sp3).reset();
    
    if (!*ts_sp3)
    {
        std::cout << "ts_sp3's underlying is now indeed NULL\n\n";
    }
     
    return 0;
}

/*
OUTPUT
#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <boost/operators.hpp>

// Author: @mbhola

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
        
        a. Pass (to the thread_safe_ptr ctor) no arguments / all arguments necessary to invoke the default / user defined ctor of the underlying.
        b. Pass (to the thread_safe_ptr ctor) an lvalue / rvalue reference to the underlying. As usual, lvalues would be copied in, and rvalues would be moved in.
        c. Invoke thread_safe_ptr's copy construction or copy assignment. 
        
        The thread_safe_ptr object thus constructed shall aggregate a shared_ptr to the underlying. As such, each such thread_safe_ptr object shall *always* have an 
        underlying, and clients need not check them for NULL prior to dereferencing. 
        
        As aforementioned in # c, it is possible to copy a thread_safe_ptr object. Each such copy shall contain a shared_ptr to the *same* underlying, and would provide 
        thread-safe access to that *same* underlying.
        
        It isn't possible to move a thread_safe_ptr object, though. Why? If move semantics are supported, the moved-from thread_safe_ptr object shall 
        lose its underlying, and thus couldn't be dereferenced. This would necessitate the client code to first check for NULL prior to dereferencing. 
    
     2. A *wrapper*, such as a shared_ptr<T>, or an IRef. That is, something that has an underlying of its own, and could be detected at compile time via the 
        availability of an overloaded indirection operator (operator->()).
        
        As an aside, note that as per classic C++ language rules, the result of an indirection should either result in a raw pointer, 
        or should result in an object of a class that itself overloads the indirection operator. The process continues until the compiler 
        arrives at a raw pointer. If not, the compile emits an error.
        
        For such types, a thread_safe_ptr could be created as follows:
        
        a. Pass (to the thread_safe_ptr ctor) no arguments / all arguments necessary to invoke the default / user defined ctor of the underlying wrapper.
        b. Pass (to the thread_safe_ptr ctor) an rvalue reference to the underlying, which would be moved in. Note that lvalue references are disallowed here. More on this below.
        c. Invoke thread_safe_ptr's copy/move construction or copy/move assignment.
        
        For such *wrapper* underlyings, the thread_safe_ptr object shall *directly* aggregate them. Note that we would like to access both the 
        underlying wrapper, and the underlying wrapper's underlying. For example, if the underlying wrapper is shared_ptr<Foo>, we would like access the public interface 
        of both shared_ptr<Foo> (for example, reset()), and the underlying Foo as well! The former requirement implies that clients of such thread_safe_ptr objects should be prepared 
        to check the wrapper underlying to be NULL prior to accessing the real underlying. This also implies that such thread_safe_ptr objects are both copyable and moveable.
        
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
    // it is a class template whose instantiation(s) provide proxy objects (on the fly) 
    // every time a thread safe object is accessed with an intent to access the underlying
    // it locks (explicitly) the associated mutex in its ctor and unlocks (implicitly) the same in its destructor 
    // once constructed, it allows for indirection to the underlying
    // it also provides for an implicit conversion to the underlying
    
    // primary template
    template<
        typename ResourceT,
        typename requested_lock_t>
    class proxy
    {
        ResourceT * const ptr{nullptr};
        requested_lock_t lock{};

    public:
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceT * const p, mutex_t& mtx)
        : ptr(p), lock(mtx) {}

        proxy(proxy&& rhs)
        : ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;

        // overload the indirection operator to reach out to the underlying 
        ResourceT* operator->() { return ptr; }
        ResourceT const* operator->() const { return ptr; }

        // overload the access operator to reach out to the underlying        
        ResourceT& operator*() { return *ptr; }
        ResourceT const& operator*() const { return *ptr; }
    };

public:

    // default constructing a thread_safe_ptr assumes that it is possible to default construct the underlying
    thread_safe_ptr()
    : ptr(std::make_unique<Resource>()), mtx(std::make_shared<mutex_t>()) {}

    // universal constructor
    // intended to invoke a viable ctor of the underlying 
    //
    // [SUBTLE]
    // this constructor would shadow the normal copy constructor while trying to create a copy of a thread_safe_ptr object
    // thus disabled for such scenarios
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!std::is_same<std::decay_t<T>, thread_safe_ptr>::value>>
    thread_safe_ptr(T&& t, Args&&... args)
    : ptr(std::make_unique<Resource>(std::forward<T>(t), std::forward<Args>(args)...)), mtx(std::make_shared<mutex_t>()) {}
    
    // provide copy semantics
    thread_safe_ptr(thread_safe_ptr const& source) = default;
    thread_safe_ptr& operator=(thread_safe_ptr const&) = default;

    // *don't* provide move semantics
    thread_safe_ptr(thread_safe_ptr const&&) = delete;
    thread_safe_ptr& operator=(thread_safe_ptr&&) = delete;
    
    // implement the *BasicLockable* interface
    // enables thread_safe_ptr objects to be used in the std::lock(...) API
    // to acquire a lock on them (to provide transactionable semantics) without risking a deadlock
    void lock() { mtx->lock(); }
    bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    auto operator->() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
};

// thread_safe_ptr specialized for *wrapper* underlyings
// note that we would like to access the public interface of both the wrapper underlying, as well as the wrapper underlying's underlying
template<
    typename Resource,
    typename mutex_t,
    typename lock_t>
class thread_safe_ptr<Resource, mutex_t, lock_t, true>
{    
    Resource res;                  // the underlying (note direct aggregation)
    std::shared_ptr<mutex_t> mtx;  // the protection
        
    template<
        typename ResourceT,
        typename requested_lock_t>
    class proxy
    {
        ResourceT * const ptr{nullptr};
        requested_lock_t lock {};

    public:
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceT * const p, mutex_t& mtx)
        : ptr(p), lock(mtx) {}

        proxy(proxy&& rhs)
        : ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;
        
        // [SUBTLE]
        // *wrapper* underlyings may get reset
        // thus, we need to provide a way for the clients to check for the same prior to dereferencing
        operator bool() const 
        {
            return (ptr && *ptr) ? true : false;
        }

        // (special) overloaded indirection to the wrapper underlying's underlying
        // this let's the client code to access the public interface of the wrapper underlying's underlying 
        auto* operator->() { return ptr->operator->(); }
        auto const* operator->() const { return ptr->operator->(); }

        // overload the access operator to reach out to the *wrapper* underlying        
        // this lets the clients to access the public interface of the wrapper underlying itself
        ResourceT& operator*() { return *ptr; }
        ResourceT const& operator*() const { return *ptr; }
    };
        
public:

    // universal constructor
    // intended to invoke a viable ctor of the underlying wrapper
    // disabled for construction via an lvalue reference to the underlying wrapper
    //
    // [SUBTLE]
    // this constructor would shadow the normal copy constructor while trying to create a copy of a thread_safe_ptr object
    // thus disabled for such scenarios
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!(std::is_lvalue_reference<T>::value && std::is_same<Resource, std::decay_t<T>>::value)>,
        typename = std::enable_if_t<!std::is_same<std::decay_t<T>, thread_safe_ptr>::value>>
    thread_safe_ptr(T&& t, Args&&... args)
    : res(std::forward<T>(t), std::forward<Args>(args)...), mtx(std::make_shared<mutex_t>()) {}
    
    // provide copy semantics
    thread_safe_ptr(thread_safe_ptr const& source) = default;
    thread_safe_ptr& operator=(thread_safe_ptr const&) = default;
    
    // provide move semantics
    thread_safe_ptr(thread_safe_ptr const&&) = delete;
    thread_safe_ptr& operator=(thread_safe_ptr&&) = delete;
    
    // implement the *BasicLockable* interface
    // enables thread_safe_ptr objects to be used in the std::lock(...) API
    // to acquire a lock on them (to provide transactionable semantics) without risking a deadlock
    void lock() { mtx->lock(); }
    bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    auto operator->() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    
    auto get() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const get() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
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
    // thread_safe_ptr<int> supports increment-assignment operator
    // since the underlying int supports the same
    // note double dereference to access the underlying by reference
    **safeInt += 42;

    // thread safe singular operation
    // indirection is *natural*; just use operator-> on the thread_safe_ptr, and it redirects to the underlying
    safeFoo->doSomething1();

    // thread safe transactional semantics
    // thread_safe_ptr implements the BasicLockable interface
    safeFoo.lock();
    safeFoo->doSomething2();
    safeFoo->doSomething3();
    safeFoo.unlock();
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeFoo);
        safeFoo->doSomething2();
        safeFoo->doSomething3();
    }    
    */
}

void f2()
{
    // thread_safe_ptr<int> supports increment-assignment operator
    // since the underlying int supports the same
    **safeInt += 42;

    // thread safe singular operation
    safeFoo->doSomething2();

    // thread safe transactional semantics
    // thread_safe_ptr implements the BasicLockable interface
    safeFoo.lock();
    safeFoo->doSomething2();
    safeFoo->doSomething3();
    safeFoo.unlock();
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeFoo);
        safeFoo->doSomething2();
        safeFoo->doSomething3();
    }    
    */
}

// either f3 or f4 populates safeMap_copy
// depending on which thread successfully acquires locks on *both* safeMap and safeMap_copy first

// thread_safe_ptr<std::map> allows for assignment
// since the underlying std::map supports the same

// [SUBTLE]
// we need to acquire a lock on the underlying shared resources (std::map) of both safeMap and safeMap_copy
// but different threads may try to acquire those locks in different orders which is a classic recipe for a deadlock!
// for example, note that f3() and f4() below acquire locks in different orders
// we need to acquire these locks atomically
// thankfully, std::lock() allows just that!

void f3()
{
    
    std::lock(safeMap, safeMap_copy); // transactional semantics

    if (safeMap_copy->empty())
    {
        // note double dereference to access the underlying by reference
        **safeMap_copy = **safeMap;
    }

    safeMap.unlock();
    safeMap_copy.unlock();
        
    /*
    Since C++17
    {
        std::scoped_lock lock(safeMap, safeMap_copy); // transactional semantics

        if (safeMap_copy->empty())
        {
            **safeMap_copy = **safeMap;
        }
    }
    */
}

void f4()
{    
    std::lock(safeMap_copy, safeMap); // transactional semantics; note different order of lock acquisition than in f3()

    if (safeMap_copy->empty())
    {
        **safeMap_copy = **safeMap;
    }

    safeMap.unlock();
    safeMap_copy.unlock();
   
    /*
    Since C++17
    {
        std::scoped_lock lock(safeMap_copy, safeMap); // transactional semantics

        if (safeMap_copy->empty())
        {
            **safeMap_copy = **safeMap;
        }
    }
    */
}

int main()
{
    std::thread t1(f1);
    std::thread t2(f2);

    t1.join();
    t2.join();

    // thread_safe_ptr<int> allows access to the underlying shared resource (int) via dereferencing
    // this is guaranteed to print 126 since increments happen in a thread safe manner in threads t1 and t2
    std::cout << **safeInt << '\n';

    // thread_safe_ptr<Foo> allows for transparent indirection for (public) member access
    std::cout << safeFoo->c << '\n';

    // thread_safe_ptr<std::map> allows for subscripting as the underlying
    // shared resource (a std::map) supports the same
    (**safeMap)[1] = 1;
    (**safeMap)[2] = 2;

    std::cout << (**safeMap)[1] << '\n';
    std::cout << (**safeMap)[2] << '\n';

    std::thread t3(f3);
    std::thread t4(f4);

    t3.join();
    t4.join();

    // safeMap_copy got populated in a thread safe manner
    // in either thread t3 or thread t4
    std::cout << (**safeMap_copy)[1] << '\n';
    std::cout << (**safeMap_copy)[2] << '\n';

    // thread_safe_ptr<std::string> allows for comparisons
    // as the underlying shared resource (std::string) supports the same
    // std::lock used here for transactional semantics
    {
        std::lock(safeStr1, safeStr2);
        std::cout << std::boolalpha << (**safeStr1 > **safeStr2) << '\n';
        std::cout << std::boolalpha << (**safeStr1 != **safeStr2) << '\n';
        safeStr1.unlock();
        safeStr2.unlock();
    }
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeStr1, safeStr2);
        std::cout << std::boolalpha << (*safeStr1 > *safeStr2) << '\n';
        std::cout << std::boolalpha << (*safeStr1 != *safeStr2) << '\n';
    }
    */
    
    // sp: a *wrapper* underlying
    std::shared_ptr<int> sp(new int(9999));
    
    // ERROR
    // cannot create thread_safe objects from lvalues of wrapper underlyings
    //thread_safe_ptr<std::shared_ptr<int>> ts_sp{sp}; 
    
    // OK
    // can create thread_safe_ptr objects from rvalues of wrapper underlyings
    thread_safe_ptr<std::shared_ptr<int>> ts_sp1{std::move(sp)};
    
    // NOTE *triple* indirection to access the wrapper underlying's underlying
    ***ts_sp1 *= 2;
    std::cout << ***ts_sp1 << '\n';
    
    // OK
    // can create thread_safe_ptr objects from rvalues of wrapper underlyings
    thread_safe_ptr<std::shared_ptr<Foo>> ts_sp2{std::make_shared<Foo>()}; 
    
    // indirection is *natural*; just use operator->() on the thread_safe_ptr, 
    // and it redirects all the way to the wrapper underlying's underlying
    ts_sp2->doSomething1();
    
    // OK
    // can create copies of thread_safe_ptrs
    thread_safe_ptr<std::shared_ptr<int>> ts_sp3{ts_sp1};
    
    // NOTE *triple* indirection to access the wrapper underlying's underlying
    ***ts_sp3 *= 2;   
    std::cout << ***ts_sp3 << '\n';
    
    // NOTE *double* indirection to the wrapper underlying itself
    (**ts_sp3).reset();
    
    if (!*ts_sp3)
    {
        std::cout << "ts_sp3's underlying is now indeed NULL\n\n";
    }
     
    return 0;
}

/*
OUTPUT
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

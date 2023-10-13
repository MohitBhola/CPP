#include <mutex>
#include <shared_mutex>
#include <memory>
#include <iostream>
#include <type_traits>
#include <vector>
#include <map>
#include <thread>

using namespace std;

template <typename T, typename = std::void_t<>>
struct disable_if_indirection : std::false_type
{};

template <typename T>
struct disable_if_indirection<T, std::void_t<decltype(std::declval<T>().operator->())>> : std::true_type
{};

template <typename Resource, bool = disable_if_indirection<Resource>::value>
class thread_safe {

    // all thread_safe instantiations are friends
    template<typename, bool>
    friend class thread_safe;
    
    // the underlying
    std::shared_ptr<Resource> ptr{};
    
    // the protection
    std::shared_ptr<std::shared_mutex> mtx{};
    
    // for unique_lock requests, ResourceT = std::decay_t<Resource> and lock_t = unique_lock<std::shared_mutex>
    // for shared_lock requests, ResourceT = const std::decay_t<Resource> and lock_t = shared_lock<std::shared_mutex>
    template <typename ResourceT, typename lock_t>
    class proxy {
        
    public:
    
        using ResourceType = ResourceT;
        
        mutable ResourceType* pUnderlying{nullptr};
        mutable lock_t lock{};

        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        template <typename LockStrategy>
        proxy(ResourceType* p, std::shared_mutex& mtx, LockStrategy lockStrategy)
        : lock(mtx, lockStrategy) /* HERE */ {
            
            // when LockStrategy = std::defer_lock_t, lock is not acquired
            // when LockStrategy = std::try_lock_t, lock is attempted but it may not get acquired
            // when LockStrategy = std::adopt_lock_t, lock is already in place
            // hold the pointer to the shared_object only if the lock has an associated mutex and has acquired ownership of it
            if (owns_lock()) {
                pUnderlying = p;
            }
        }
        
        // when the proxy object is created, acquire a lock on the underlying *explicitly*
        proxy(ResourceType* p, std::shared_mutex& mtx)
        : pUnderlying(p), lock(mtx) /* HERE */ {
        
            // when LockStrategy = std::defer_lock_t, lock is not acquired
            // when LockStrategy = std::try_lock_t, lock is attempted but it may not get acquired
            // when LockStrategy = std::adopt_lock_t, lock is already in place
            // hold the pointer to the shared_object only if the lock has an associated mutex and has acquired ownership of it
            if (owns_lock()) {
                pUnderlying = p;
            }
        }

        // move enabled
        proxy(proxy&& rhs)
        : pUnderlying(rhs.pUnderlying), lock(std::move(rhs.lock)) {
            rhs.pUnderlying = nullptr;
        }
        
        // copy disabled
        proxy(const proxy& rhs) = delete;
        
        // when the proxy object is destroyed, release the lock on the underlying *implicitly*
        ~proxy() noexcept = default;

        // overload the indirection operator to reach out to the underlying
        ResourceType* operator->() const {
            return pUnderlying;
        }

        // overload the dereference operator to reach out to the underlying
        ResourceType& operator*() const {
            return *pUnderlying;
        }
        
        // client code may choose to express in code that the lock is being released
        // any further attemp to use the proxy would be an abomination
        void unlock() const {
            lock.unlock();
            pUnderlying = nullptr;
        }
        
        bool owns_lock() const {
            return lock.owns_lock();
        }
    };

public:
    
    using ResourceType = std::decay_t<Resource>;
	
    // default constructing a thread_safe<Resource> object requires the type Resource to be default constructible
    template <typename T = ResourceType, typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    thread_safe()
    : ptr(std::make_shared<ResourceType>()), mtx(std::make_shared<std::shared_mutex>()) {}

    // universal ctor
    // intended to construct the underlying by invoking a viable ctor of the underlying using the parameters of this universal ctor
    // such viable ctors include normal ctors taking multiple arguments as well as copy / move ctors
    //
    // [SUBTLE]
    // this ctor would shadow the normal copy ctor of thread_safe while trying to create a copy of a thread_safe
    // thus disabled for such scenarios
    template <typename T, typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, thread_safe>>>
    thread_safe(T&& t, Args&&... args)
    : ptr(std::make_shared<ResourceType>(std::forward<T>(t), std::forward<Args>(args)...)), mtx(std::make_shared<std::shared_mutex>()) {}
    
    thread_safe(std::unique_ptr<ResourceType> pResource)
    : ptr(std::move(pResource)), mtx(std::make_shared<std::shared_mutex>()) {}
    
    // copy enabled
    thread_safe(const thread_safe& source) = default;
    thread_safe& operator=(const thread_safe&) = default;

    // move enabled
    thread_safe(thread_safe&&) = default;
    thread_safe& operator=(thread_safe&&) = default;
    
    // generic converting ctor
    template <typename T>
    thread_safe(thread_safe<T>& other)
    : ptr(other.ptr), mtx(other.mtx) {}
    
    // implement the *Lockable* and *SharedLockable* named requirements
    // enables thread_safe objects to be used with the std::lock(...) API and the std::scoped_lock construct
    // (which help to acquire a lock on multiple thread_safe objects in different threads without risking a deadlock)
    void lock() const {
        mtx->lock();
    }

    void lock_shared() const {
        mtx->lock_shared();
    }
    
    bool try_lock() const {
        return mtx->try_lock();
    }
    
    bool try_lock_shared() const {
        return mtx->try_lock_shared();
    }
    
    void unlock() const {
        mtx->unlock();
    }
    
    void unlock_shared() const {
        mtx->unlock_shared();
    }

    // a unique_lock request should only come from a non-const thread_safe object
    // hence this api is non-const
    auto unique_lock() {
        return proxy<ResourceType, std::unique_lock<std::shared_mutex>>(ptr.get(), *mtx);
    }
    
    template <typename LockStrategy, typename = std::enable_if_t<!std::is_same_v<std::defer_lock_t, std::decay_t<LockStrategy>>>>
    auto unique_lock(LockStrategy lockStrategy) {
        return proxy<ResourceType, std::unique_lock<std::shared_mutex>>(ptr.get(), *mtx, lockStrategy);
    }

    // a shared_lock request should only come from a const thread_safe object
    // hence this api is const
    auto shared_lock() const {
        return proxy<const ResourceType, std::shared_lock<std::shared_mutex>>(ptr.get(), *mtx);
    }
    
    template <typename LockStrategy, typename = std::enable_if_t<!std::is_same_v<std::defer_lock_t, std::decay_t<LockStrategy>>>>
    auto shared_lock(LockStrategy lockStrategy) const {
        return proxy<const ResourceType, std::shared_lock<std::shared_mutex>>(ptr.get(), *mtx, lockStrategy);
    }
};

class Base {
  
public:

    Base() {std::cout << "Base Ctor" << std::endl;}
    virtual ~Base() noexcept = default;
};

class Derived : public Base {
  
public:

    Derived() {std::cout << "Derived Ctor" << std::endl;}
    virtual ~Derived() noexcept = default;
};

/*
struct Bar {
    Bar() = default;
    int ival = 0;
};

struct Thing {
    thread_safe<Bar> fBar;
    void DoSomethingConst() const {
        auto proxy = fBar.unique_lock(); // <-- compile error
    }
    static void ModifyBar(Bar& bar)
    {
        bar.ival = 42;
    }
    void DoSomethingNonConst() {
        auto proxy = fBar.shared_lock();
        ModifyBar(*proxy); // <-- compile error
    }
};
*/

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

void f1()
{
    {
        // in order to do anything with the underlying, clients first need to invoke the Lock() API on the synchronized ptr
        // otherwise, the code won't compile
        // this would prevent the class of bugs arising out of having locked a wrong mutex or forgotten to lock a mutex at all
        // while trying to access the shared resource
        auto intRef = safeInt.unique_lock();
        
        // once we have obtained the proxy to an underlying, clients need to dereference it to do a thread safe access to that underlying
        *intRef += 42;
    } // the proxy goes out of scope here and releases the lock on the underlying
      // this covers both normal returns and exceptions as well

    // performing a thread safe singular operation
    // indirection is *natural*; just use operator-> on the proxy returned by the thread_safe_ptr, and it seamlessly redirects to the underlying
    {
        auto fooRef = safeFoo.unique_lock();
        fooRef->doSomething1();
    }
    
    // performing a thread safe transaction
    {
        auto fooRef = safeFoo.unique_lock();
        fooRef->doSomething2();
        fooRef->doSomething3();
    }
}

void f2()
{
    {
        // accessing the safeInt in a thread safe manner along with such an access in function f1()
        auto intRef = safeInt.unique_lock();
        *intRef += 42; // a thread safe increment
    }

    // a thread safe singular operation, as in function f1()
    {
        auto fooRef = safeFoo.unique_lock();
        fooRef->doSomething2();
    }

    // a thread safe transaction, as in function f1()
    {
        auto fooRef = safeFoo.unique_lock();
        fooRef->doSomething2();
        fooRef->doSomething3();
    }
}

// works with library types
thread_safe<std::map<int, int>> safeMap{};
thread_safe<std::map<int, int>> safeMap_copy{};

void f3()
{
    std::scoped_lock aLock(safeMap, safeMap_copy);

    {
        auto mapRef = safeMap.unique_lock(std::adopt_lock);
        auto mapCopyRef = safeMap_copy.unique_lock(std::adopt_lock);
     
        if (mapCopyRef->empty())
        {
            *mapCopyRef = *mapRef;
        }
    }
}
 
void f4()
{
    std::scoped_lock aLock(safeMap_copy, safeMap);

    {
        auto mapRef = safeMap.unique_lock(std::adopt_lock);
        auto mapCopyRef = safeMap_copy.unique_lock(std::adopt_lock);
     
        if (mapCopyRef->empty())
        {
            *mapCopyRef = *mapRef;
        }
    }
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
    std::cout << *safeInt.unique_lock() << '\n';
    
    // thread_safe_ptr<Foo> allows for transparent indirection to access the public API of the underlying
    std::cout << safeFoo.unique_lock()->c << '\n';

    // populate safeMap in a thread safe manner
    {
        auto mapRef = safeMap.unique_lock();
        
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
        auto mapCopyRef = safeMap_copy.unique_lock();
        
        std::cout << (*mapCopyRef)[1] << '\n';
        std::cout << (*mapCopyRef)[2] << '\n';
    }

    return 0;
}

/*
OUTPUT in XCode 13.2.1
error: module importing failed: invalid pathname
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
Program ended with exit code: 0
*/

#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <type_traits>

using namespace std;

// storage for the handleID and its associated resource release function
// MeyersSingleton
auto& GetHandleReleaseFunctions()
{
    static std::unordered_map<uintptr_t, std::function<void()>> handleReleaseFunctions;
    return handleReleaseFunctions;
}

// detect whether the underlying is a shared_ptr<>, unique_ptr<>, IRef etc
// specifically, we detect whether the underlying resource provides for indirection
// this information would be leveraged to provide a custom indirection for handles to such types
// see more comments around this elsewhere
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
class Handle
{
    // a template parameter at the class scope hides a similarly named template parameter of the class template
    // so, using ResourceT
    template <typename ResourceT>
    static std::unordered_map<uintptr_t, ResourceT> resources;
    
    // a mutex isn't copyable or moveable
    // encapsulating the mutex in a shared_ptr allows us to effectively
    // capture it in the handle's resource release function
    std::shared_ptr<mutex_t> mMutex {std::make_shared<mutex_t>()};
    
    // the surrogate
    // allows thread safe indirection to the underlying
    // also allows implicit conversion to bool to check if the underlying actually exists or not
    template
    <
        typename ResourceT,
        typename requested_lock_t,
        bool = EnableIfIndirection<ResourceT>::value
    >
    class proxy
    {
        ResourceT* pUnderlying {nullptr};
        requested_lock_t lock;
        uintptr_t handleID{};
    
    public:
    
        // by default, the underlying isn't available and thus the lock on the underlying isn't acquired
        proxy() = default;
        
        // if the underlying is present, acquire the lock on the underlying when the proxy gets constructed
        proxy(ResourceT* p, mutex_t& mtx, uintptr_t keyID)
        : pUnderlying(p), lock(mtx), handleID(keyID)
        {}
        
        proxy(proxy&& rhs)
        : pUnderlying(std::move(rhs.pUnderlying)), lock(std::move(rhs.lock)), handleID(std::move(rhs.handleID)) {}
        
        // implicitly release the lock when the proxy gets destroyed
        ~proxy() noexcept = default;
        
        operator bool() const { return static_cast<bool>(pUnderlying); }
        
        // for underlying types that support indirection (that is, who are wrappers themselves), reach out to their underlying
        // why do this?
        // to provide for a natural, intuitive syntax for the client code
        // for example, to allow for mHandle->DoSomething() even if the underlying type is an IRef,
        // whose underlying type is some class Foo that provides a public API DoSomething()
        auto* operator->() { return pUnderlying->operator->(); }
        auto const* operator->() const { return pUnderlying->operator->(); }
        
        ResourceT& operator*() { return *pUnderlying; }
        ResourceT const& operator*() const { return *pUnderlying; }
    };
    
    // the surrogate specialized for *normal* underlying types
    // that is, regular types that aren't wrappers
    template
    <
        typename ResourceT,
        typename requested_lock_t
    >
    class proxy<ResourceT, requested_lock_t, false>
    {
        ResourceT* pUnderlying{nullptr};
        requested_lock_t lock;
        uintptr_t handleID{};
    
    public:
    
        // by default, the underlying isn't available and thus the lock on the underlying isn't acquired
        proxy() = default;
        
        // if the underlying is present, acquire the lock on the underlying when the proxy gets constructed
        proxy(ResourceT* p, mutex_t& mtx, uintptr_t keyID)
        : pUnderlying(p), lock(mtx), handleID(keyID)
        {}
        
        proxy(proxy&& rhs)
        : pUnderlying(std::move(rhs.pUnderlying)), lock(std::move(rhs.lock)), handleID(std::move(rhs.handleID)) {}
        
        // implicitly release the lock when the proxy gets destroyed
        ~proxy() noexcept = default;
        
        operator bool() const { return static_cast<bool>(pUnderlying); }
        
        // indirection is simpler for *normal* types that aren't wrappers
        // note that we just return the raw pointer to the underlying
        ResourceT* operator->() { return pUnderlying; }
        ResourceT const* operator->() const { return pUnderlying; }
        
        ResourceT& operator*() { return *pUnderlying; }
        ResourceT const& operator*() const { return *pUnderlying; }
    };
    
    template <typename... ArgumentsToConstructResource>
    std::enable_if_t<(sizeof...(ArgumentsToConstructResource) > 0)>
    insertUnderlying(lock_t& lock, ArgumentsToConstructResource&&... argumentsToConstructResource)
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        // the handle might already be *loaded*
        // in that case, the client might be trying to *re-load* the handle
        // so, remove the existing underlying first
        removeUnderlying(lock);
        
        // now insert the desired underlying (via perfect forwarding)
        // the underlying gets constructed *in-place* within the static storage
        resources<Resource>.emplace(std::piecewise_construct, std::forward_as_tuple(keyID), std::forward_as_tuple(argumentsToConstructResource...));
        
        // register the handle release function that would get invoked during purge
        GetHandleReleaseFunctions().insert(std::make_pair(keyID, [=]()
        {
            std::lock_guard<std::recursive_mutex> guard(*mMutex);
            
            // ensure that the handle's underlying still exists in the static storage
            // this check *feels* redundant, but nevertheless is safe
            if (resources<Resource>.find(keyID) != resources<Resource>.end())
            {
                // wiping out the handle's entry from the static storage is enough to destroy it
                (void)resources<Resource>.erase(keyID);
            }
        }));
    }
    
    void removeUnderlying(lock_t&)
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        auto iter = GetHandleReleaseFunctions().find(keyID);
        if (iter != std::end(GetHandleReleaseFunctions()))
        {
            auto const& handleReleaseCallable = iter->second;
            
            // invoke the handle's release function
            // this would wipe out the handle's entry from the static storage
            handleReleaseCallable();
            (void)GetHandleReleaseFunctions().erase(iter);
        }
        
        // redundant, but nevertheless safe
        // ensure that handle's entry has indeed been wiped out from the static storage
        if (resources<Resource>.find(keyID) != resources<Resource>.end())
        {
             (void)resources<Resource>.erase(keyID);
        }
    }
    
public:

    Handle() = default;
    
    // clients must adhere to the following usage:
    /*
     * {
     *     auto fooRef = mFooHandle.lock();
     *     if (fooRef)
     *     {
     *         fooRef->DoSomething();
     *     }
     * }
     */
    auto lock()
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        return (resources<Resource>.find(keyID) == resources<Resource>.end()) ?
            proxy<Resource, lock_t>() :
            proxy<Resource, lock_t>(std::addressof(resources<Resource>.at(keyID)), *mMutex, keyID);
    }
    
    auto const lock() const
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        return (resources<Resource>.find(keyID) == resources<Resource>.end()) ?
            proxy<Resource, lock_t>() :
            proxy<Resource, lock_t>(std::addressof(resources<Resource>.at(keyID)), *mMutex, keyID);
    }
    
    // clients shall invoke this API to *load* the handle with the underlying
    // they need to pass all the arguments that could be used to invoke an appropriate constructor of the underlying
    // this includes the default constructor, user defined constructors, and copy/move constructors
    template <typename... ArgumentsToConstructResource>
    std::enable_if_t<(sizeof...(ArgumentsToConstructResource) > 0)>
    reset(ArgumentsToConstructResource&&... argumentsToConstructResource)
    {
        lock_t lock(*mMutex);
        insertUnderlying(lock, argumentsToConstructResource...);
    }
    
    // allow for clients to explicitly free the underlying
    void reset()
    {
        lock_t lock(*mMutex);
        removeUnderlying(lock);
    }
    
    Handle& operator=(Handle const& other)
    {
        if (this == &other)
        {
            return *this;
        }
        
        lock_t lock(*mMutex);
        reset(*(other.lock()));
        
        return *this;
    }
};

template <typename Resource, typename mutex_t, typename lock_t>
template <typename ResourceT>
std::unordered_map<uintptr_t, ResourceT> Handle<Resource, mutex_t, lock_t>::resources;

// specialized for pointers
template 
<
    typename Resource,
    typename mutex_t,
    typename lock_t
> 
class Handle<Resource*, mutex_t, lock_t>
{       
    template <typename ResourceT>
    static std::unordered_map<uintptr_t, ResourceT*> resourcesToBeDelete;
    
    std::shared_ptr<mutex_t> mMutex {std::make_shared<mutex_t>()};
    
    // the surrogate
    // allows thread safe indirection to the underlying 
    // also allows implicit conversion to bool to check if the underlying actually exists or not
    template 
    <
        typename ResourceT,
        typename requested_lock_t
    >
    class proxy
    {
        ResourceT* pUnderlying {nullptr};
        requested_lock_t lock;
    
    public:
    
        // by default, the underlying isn't available and thus the lock on the underlying isn't acquired
        proxy() = default;
        
        // if the underlying is present, acquire the lock on the underlying when the proxy gets constructed
        proxy(ResourceT* p, mutex_t& mtx)
        : pUnderlying(p), lock(mtx) {}
        
        proxy(proxy&& rhs)
        : pUnderlying(std::move(rhs.pUnderlying)), lock(std::move(rhs.lock)) {}
        
        // implicitly release the lock when the proxy gets destroyed
        ~proxy() noexcept = default;
        
        operator bool() const { return static_cast<bool>(pUnderlying); }
        
        auto* operator->() { cout << "HERE5" << '\n'; return pUnderlying; }
        auto const* operator->() const { cout << "HERE6" << '\n'; return pUnderlying; }
        
        ResourceT& operator*() { cout << "HERE5" << '\n'; return *pUnderlying; }
        ResourceT const& operator*() const { cout << "HERE6" << '\n'; return *pUnderlying; }
    };
    
    template <typename... ArgumentsToConstructResource>
    std::enable_if_t<(sizeof...(ArgumentsToConstructResource) > 0)> 
    insertUnderlying(lock_t& lock, ArgumentsToConstructResource&&... argumentsToConstructResource) 
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        // the handle might already be *loaded*
        // in that case, the client might be trying to *re-load* the handle
        // so, remove the existing underlying first
        removeUnderlying(lock);
        
        // now insert the desired underlying (via perfect forwarding)
        // the underlying gets constructed *in-place* within the static storage
        resourcesToBeDelete<Resource>.emplace(std::piecewise_construct, std::forward_as_tuple(keyID), std::forward_as_tuple(argumentsToConstructResource...));
        
        // register the handle release function that would get invoked during purge
        GetHandleReleaseFunctions().insert(std::make_pair(keyID, [=]()
        {
            std::lock_guard<std::recursive_mutex> guard(*mMutex);
                        
            // ensure that the handle's underlying still exists in the static storage
            // this check *feels* redundant, but nevertheless is safe
            if (resourcesToBeDelete<Resource>.find(keyID) != resourcesToBeDelete<Resource>.end()) 
            {
                cout << "handle release function 2.1 " << '\n';

                // in the primary template, it sufficed to just erase the entry from the map
                // but in the specialization for pointers, we need to *delete* the underlying as well
                delete resourcesToBeDelete<Resource>[keyID];
                (void)resourcesToBeDelete<Resource>.erase(keyID);
            }
        }));
    }
    
    void removeUnderlying(lock_t&)
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        auto iter = GetHandleReleaseFunctions().find(keyID); 
        if (iter != std::end(GetHandleReleaseFunctions()))
        {
            auto const& handleReleaseCallable = iter->second;
            
            // invoke the handle's release function
            // this would *delete* the underlying and wipe out the handle's entry from the static storage
            handleReleaseCallable();
            (void)GetHandleReleaseFunctions().erase(iter);
        }
        
        // redundant, but nevertheless safe
        // ensure that the underlying has been *deleted* and that the handle's entry has been wiped out from the static storage
        if (resourcesToBeDelete<Resource>.find(keyID) != resourcesToBeDelete<Resource>.end())
        {
            delete resourcesToBeDelete<Resource>[keyID];
            (void)resourcesToBeDelete<Resource>.erase(keyID);
        }
    }
    
public:

    Handle() = default;
    
    // clients must adhere to the following usage:
    /*
     * {
     *     auto fooRef = mFooHandle.lock();
     *     if (fooRef)
     *     {
     *         fooRef->DoSomething();
     *     }
     * }
     */
    auto lock() 
    {
        auto keyID = reinterpret_cast<uintptr_t>(this);
        
        return (resourcesToBeDelete<Resource>.find(keyID) == resourcesToBeDelete<Resource>.end()) ? 
            proxy<Resource, lock_t>() : 
            proxy<Resource, lock_t>(resourcesToBeDelete<Resource>[keyID], *mMutex);
    }
    
    // clients shall invoke this API to *load* the handle with the underlying
    // they need to pass all the arguments that could be used to invoke an appropriate constructor of the underlying
    // this includes the default constructor, user defined constructors, and copy/move constructors
    template <typename... ArgumentsToConstructResource>
    std::enable_if_t<(sizeof...(ArgumentsToConstructResource) > 0)> 
    reset(ArgumentsToConstructResource&&... argumentsToConstructResource) 
    {
        lock_t lock(*mMutex);
        insertUnderlying(lock, argumentsToConstructResource...);
    }
    
    // allow for clients to explicitly free the underlying
    void reset() 
    {
        lock_t lock(*mMutex);
        removeUnderlying(lock);
    }
};

template <typename Resource, typename mutex_t, typename lock_t>
template <typename ResourceT>
std::unordered_map<uintptr_t, ResourceT*> Handle<Resource*, mutex_t, lock_t>::resourcesToBeDelete;

struct Foo
{
    int i = 42;
    
    Foo() = default;
    
    Foo(int initValue) : i(initValue) {}
    
    Foo(Foo const& other) 
    {
        i = other.i;
    }
    
    ~Foo()
    {
        cout << "Foo dtor" << '\n';
    }
    
    void print() const
    {
        cout << i << '\n';
    }
};

int main()
{ 
    // underlying be shared_ptr<Foo>; supports indirection
    Handle<shared_ptr<Foo>> fooHandle1{};
    
    // RAII based usage
    // clients are supposed to *lock* the handle for *thread safe* access to the underlying 
    // doing so returns a proxy to the underlying; clients need not know the proxy's actual type
    // it should suffice to know that the proxy is move only, and primarily allows thread-safe indirection to the underlying
    // also, when the proxy goes out of scope, it releases the lock on the underlying
    // prior to indirection, the clients must ensure that the underlying actually exists
    // if they are not sure of that, they should use the proxy in a boolean context to be sure and act accordingly
    
    {
        auto fooRef1 = fooHandle1.lock();
    
        if (!fooRef1)
        {
            cout << "fooHandle1 is indeed NULL" << '\n';
            fooHandle1.reset(new Foo(101));
        }
    
        auto fooRef2 = fooHandle1.lock();
        if (fooRef2)
        {
            cout << "fooHandle1 is now loaded" << '\n';
        }
        
        fooRef2->print(); // access the underlying
    
        fooHandle1.reset(new Foo(201));
    
        auto fooRef3 = fooHandle1.lock();
        if (fooRef3)
        {
            fooRef3->print(); 
        }
    
        fooHandle1.reset();
    
        auto fooRef4 = fooHandle1.lock();    
        if (!fooRef4)
        {
            cout << "fooHandle1 is now indeed NULL\n";
        }    

        // pass *anything* that could be used to construct the underlying
        // that is, *anything* that could end up invoking the default/user-defined/copy/move/universal ctors
        fooHandle1.reset(std::make_shared<Foo>(301)); 
    
        auto fooRef5 = fooHandle1.lock();
        if (fooRef5)
        {
            fooRef5->print(); 
        }
        
        Handle<shared_ptr<Foo>> fooHandle11{};
        fooHandle11 = fooHandle1;
        auto fooRef6 = fooHandle11.lock();
        if (fooRef6)
        {
            fooRef6->print();
        }
    }
    
    cout << "=========================\n\n";
    
    // underlying be a *normal* class not providing for indirection
    Handle<Foo> fooHandle2;
    
    fooHandle2.reset(1001);
    {
        auto fooRef1 = fooHandle2.lock();
        
        if (fooRef1)
        {
            fooRef1->print();
        }
    
        fooHandle2.reset();
    
        auto fooRef2 = fooHandle2.lock();
        
        if (!fooRef2)
        {
            cout << "fooHandle2 is indeed NULL" << '\n';
        }
        
        fooHandle2.reset(2001);
        
        auto fooRef3 = fooHandle2.lock();
        
        if (fooRef3)
        {
            cout << "fooHandle2 is now loaded" << '\n';
            fooRef3->print();
        }
    
        fooHandle2.reset(Foo(3001));
    
        auto fooRef4 = fooHandle2.lock();
        
        if (fooRef4)
        {
            fooRef4->print();
        }
    }
    
    cout << "=========================\n\n";
    
    // underlying be a raw pointer to a resource
    Handle<Foo*> fooPtrHandle{};
    
    {
        auto fooPtrRef1 = fooPtrHandle.lock();
        if (!fooPtrRef1)
        {
            cout << "fooPtrHandle is indeed NULL" << endl;
            fooPtrHandle.reset(new Foo(42));
        }
        
        auto fooPtrRef2 = fooPtrHandle.lock();
        if (fooPtrRef2)
        {
            cout << "fooPtrHandle is now loaded" << endl;
            fooPtrRef2->print();
        }
    }
        
    for (auto const& pair : GetHandleReleaseFunctions())
    {
        auto const& handle = pair.first;
        auto const& handleReleaseFunction = pair.second;
        
        cout << "Invoking resource release function for handle: " << handle << '\n';
        handleReleaseFunction();
    }
        
    return 0;
}

/*
OUTPUT
fooHandle1 is indeed NULL
fooHandle1 is now loaded
101
Foo dtor
201
Foo dtor
fooHandle1 is now indeed NULL
301
301
=========================

1001
Foo dtor
fooHandle2 is indeed NULL
fooHandle2 is now loaded
2001
Foo dtor
Foo dtor
3001
=========================

fooPtrHandle is indeed NULL
fooPtrHandle is now loaded
HERE5
42
Invoking resource release function for handle: 140735949438544
Foo dtor
Invoking resource release function for handle: 140735949438480
Invoking resource release function for handle: 140735949438608
handle release function 2.1 
Foo dtor
Invoking resource release function for handle: 140735949438496
Foo dtor
*/


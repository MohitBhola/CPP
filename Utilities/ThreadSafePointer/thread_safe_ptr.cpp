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

        // when the proxy object is created, acquire a lock on the underlying explicitly as per lockStrategy
        template <typename LockStrategy>
        proxy(ResourceType* p, std::shared_mutex& mtx, LockStrategy lockStrategy)
        : lock(mtx, lockStrategy) /* HERE */ {
            
            // when LockStrategy = std::defer_lock_t, lock is not acquired
            // when LockStrategy = std::try_lock_t, lock is attempted but it may not get acquired
            // when LockStrategy = std::adopt_lock_t, lock is already in place
            // hold the pointer to the underlying only if the lock has an associated mutex and has acquired ownership of it
            if (owns_lock()) {
                pUnderlying = p;
            }
        }
        
        // for unique_lock requests, acquire an exclusive ownership of the mutex
        // if another thread is holding an exclusive lock or a shared lock on the same mutex, block execution until all such locks are released
        // with exclusive ownership of the mutex, no other lock of any kind can also be held
        //
        // for shared_lock requests, acquire shared ownership of the mutex
        // if another thread is holding the mutex in exclusive ownership, block execution until shared ownership can be acquired
        proxy(ResourceType* p, std::shared_mutex& mtx)
        : pUnderlying(p), lock(mtx) /* HERE */ {}
        
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

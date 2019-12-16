#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <boost/operators.hpp>

// Author: @mbhola

template <typename...>
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
class thread_safe
{
	/*
	
	 A thread_safe object *owns* an underlying object. There could be 2 types of underlyings:
	
	 1. A normal, regular class that isn't providing wrapper semantics. That is, something that doesn't has an underlying of its own.
	    
	    For such underlyings, a thread_safe object shall aggregate a shared_ptr to the underlying. As such, each thread_safe object shall 
	    *always* have an underlying, and clients need not check them for NULL prior to dereferencing. 
	    
	    Copies of such a thread_safe object could be made, though, and each such copy shall contain a shared_ptr to the *same* underlying, 
	    and would provide thread-safe access to the that *same* underlying.
		
		thread_safe objects aren't moveable, though. It is a conscious design choice to disallow moving thread_safe objects. 
		Rationale: if they provide move semantics, the moved-from thread_safe object shall become *empty* (NULL). This would 
		necessitate the client code to first check for NULL prior to dereferencing. This would lead to clumsy/bloated usage.
		  
		As is, every thread_safe object shall *always* have a shared_ptr to the *same* underlying, and thus clients can freely 
		dereference them in a thread-safe manner. There might be copies floating around, but each such copy shall allow 
		for thread-safe access to the *same * underlying.
	
	 2. A wrapper, such as a shared_ptr<T>, or an IRef. That is, something that has an underlying of its own, and could be detected
	    at compile time via the availability of an overloaded indirection operator (operator->()).
	    
	    As an aside, note that as per classic C++ language rules, the result of an indirection should either result in a raw pointer, 
	    or should result in an object of a class that itself overloads the indirection operator. The process continues until the compiler 
	    arrives at a raw pointer. If not, the compile emits an error.
	    
		For such *special* underlyings, the thread_safe object shall *directly* aggregate them. And shall provide special indirection 
		to forward to the (real) underlying of the underlying. 
		
		Just like for the normal case (# 1 above), each thread_safe object shall always have a wrapper that points to the *same* 
		*real* underlying. Again, copies could be made, but each such copy shall have a wrapper that points to the *same* 
		*real* underlying, and would provide thread-safe access to the *same* *real* underlying in a thread-safe manner.
		
		[SUBTLE]
		
		The ownership group needs to be a closed one. That is, there shouldn't be a *naked* wrapper anywhere else that points 
		to the *same* *real* underlying as pointed to by a group of copy of thread_safe objects, else access isn't thread-safe. 
		
		[Translation] 
		
		Disallow construction of thread_safe objects to wrapper underlyings via lvalues to such underlyings. 
		Those lvalues would defeat thread-safe access to the real underlying. 
		Why?
		Because, access to the *real* *underying* isn't threaf-safe via those lvalues.
		RValues are OK, though. We'll move them in to create a source thread_safe object, and any subsequent copies 
		of that thread_safe object shall point to the *same*nunderlying, and would allow for thread-safe access to that *same* underlying.
		
	*/
	    
    std::shared_ptr<Resource> ptr; // the underlying
    std::shared_ptr<mutex_t> mtx;  // the protection
    
    // The Surrogate
    // what's this?
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
        : private boost::operators<proxy<ResourceT, requested_lock_t>>
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

public:

    template <
        typename... Args>
    thread_safe(Args&&... args)
    : ptr(std::make_unique<Resource>(std::forward<Args>(args)...)), mtx(std::make_shared<mutex_t>()) {}
    
    // provide copy semantics
    // if a thread_safe object is created via copy semantics,
    // the copy shares the same underlying and the same associated protection
    // so, in effect, there could be several thread_safe copies of an underlying
    // in different threads, and they could access the underlying in a thread safe manner
    thread_safe(thread_safe const& source) = default;
    thread_safe& operator=(thread_safe const&) = default;

    // *don't* provide move semantics
	thread_safe(thread_safe const&&) = delete;
    thread_safe& operator=(thread_safe&&) = delete;
    
    void lock() { mtx->lock(); }
	bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    auto operator->() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(ptr.get(), *mtx);}
};

template<
    typename Resource,
    typename mutex_t,
    typename lock_t>
class thread_safe<Resource, mutex_t, lock_t, true>
{    
    Resource res;                  // the underlying
    std::shared_ptr<mutex_t> mtx;  // the protection
        
    template<
    	typename ResourceT,
        typename requested_lock_t>
    class proxy
    	: private boost::operators<proxy<ResourceT, requested_lock_t>>
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

		auto& operator*() { return ptr->operator*(); }
        auto const& operator*() const { return ptr->operator*(); }
        
        operator ResourceT&() { return ptr->operator*(); }
        operator ResourceT const&() const { return ptr->operator*(); }      
    };
        
public:

    // disallow construction via an lvalue reference to a wrapper
    template<
        typename T,
        typename... Args,
        typename = std::enable_if_t<!(std::is_lvalue_reference<T>::value &&
                                        std::is_same<Resource, std::decay_t<T>>::value)>>
    thread_safe(T&& t, Args&&... args)
    : res(std::forward<T>(t), std::forward<Args>(args)...), mtx(std::make_shared<mutex_t>()) {}
    
    // provide copy semantics
    // if a thread_safe object is created via copy semantics,
    // the copy shares the same underlying and the same associated protection
    // so, in effect, there could be several thread_safe copies of an underlying
    // in different threads, and they could access the underlying in a thread safe manner
    thread_safe(thread_safe const& source) = default;
    thread_safe& operator=(thread_safe const&) = default;
    
    void lock() { mtx->lock(); }
	bool try_lock() { return mtx->try_lock(); }
    void unlock() { mtx->unlock(); }

    auto operator->() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const operator->() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}

    auto operator*() {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
    auto const operator*() const {return proxy<Resource, lock_t>(std::addressof(res), *mtx);}
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
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeMap, safeMap_copy); // transactional semantics

        if (safeMap_copy->empty())
        {
            *safeMap_copy = *safeMap;
        }
    }
    */
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
    
    /*
    Since C++17
    {
        std::scoped_lock lock(safeMap_copy, safeMap); // transactional semantics

        if (safeMap_copy->empty())
        {
            *safeMap_copy = *safeMap;
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
    
    std::shared_ptr<int> sp(new int(9999));
    
    // ERROR
    // cannot create threa_safe objects from lvalues of wrapper types
    //thread_safe<std::shared_ptr<int>> ts_sp{sp}; 
    
    // OK
    // can only create thread_safe objects from rvalues of wrapper types
    thread_safe<std::shared_ptr<int>> ts_sp{std::move(sp)}; 
    
        
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
*/

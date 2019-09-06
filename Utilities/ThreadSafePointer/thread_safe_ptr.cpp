#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <boost/operators.hpp>

template <
	typename T,
	typename mutex_t = std::recursive_mutex,
	typename lock_t  = std::unique_lock<mutex_t>>
class thread_safe_ptr
{
	std::unique_ptr<T> ptr;
	std::unique_ptr<mutex_t> mtx;

	// the surrogate
	template <
		typename requested_lock_t>
	class proxy 
		: private boost::operators<proxy<requested_lock_t>>
	{
		T * const ptr{nullptr};
		requested_lock_t lock {};

	public:

		proxy(T * const p, mutex_t& mtx)
		: ptr(p), lock(mtx) {}

		proxy(proxy&& rhs)
		: ptr(std::move(rhs.ptr)), lock(std::move(rhs.lock)) {}

		proxy& operator=(proxy const& rhs)
		{
			*ptr = *rhs.ptr;
			return *this;
		}

		T* operator->() { return ptr; }
		T const* operator->() const { return ptr; }

		operator T&() { return *ptr; }
		operator T const&() const { return *ptr; }

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
	thread_safe(Args... args)
	: ptr(std::make_unique<T>(std::forward<Args>(args)...)), mtx(std::make_unique<mutex_t>()) {}

	void lock() {mtx->lock();}
	void unlock() {mtx->unlock();}
	auto try_lock() {return mtx->try_lock();}

	auto operator->() {return proxy<lock_t>(ptr.get(), *mtx);}
	auto const operator->() const {return proxy<lock_t>(ptr.get(), *mtx);}

	auto operator*() {return proxy<lock_t>(ptr.get(), *mtx);}
	auto const operator*() const {return proxy<lock_t>(ptr.get(), *mtx);}
};

class Foo
{
	int i{42}; 
	float f{42.0}; 
	char c{'a'}; 

public:

	 explicit Foo(int i_, float f_, char c_) : i(i_), f(f_), c(c_) {}
	 void doSomething1() {}
	 void doSomething2() {}
	 void doSomething3() {}
};

// works with fundamental types 
thread_safe_ptr<int> safeInt(42); 
 
// works with user defined types 
thread_safe_ptr<Foo> safeFoo{}; 
 
// works with library types 
thread_safe<map<int, int>> safeMap{}; 
thread_safe<map<int, int>> safeMap_copy{}; 
 
thread_safe<string> safeStr1{"abc"}; 
thread_safe<string> safeStr2{"xyz"}; 

void f1()
{
	// thread_safe_ptr<int> supports increment-assignment operator
	// since the underlying int supports the same
	*safeInt += 42; 

	// thread safe singular operation
	safeFoo->doSomething1();

	// thread safe transactional semantics
	// thread_safe_ptr implements the BasicLockable interface
	{  
		std::lock lk(safeFoo);
		safeFoo->doSomething2();
		safeFoo->doSomething3();
	}
}

void f2()
{
	// thread_safe_ptr<int> supports increment-assignment operator
	// since the underlying int supports the same
	*safeInt += 42;

	// thread safe singular operation
	safeFoo->doSomething2();

	// thread safe transactional semantics
	// thread_safe_ptr implements the BasicLockable interface
	{
		std::lock lk(safeFoo);
		safeFoo->doSomething2();
		safeFoo->doSomething3();
	}
}

// either f3 or f4 populates safeMap_copy
// depending on which thread successfully acquires locks on both safeMap and safeMap_copy first

// thread_safe_ptr<std::map> allows for assignment
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
		if (*safeMap_copy.empty())
		{
			*safeMap_copy = *safeMap;
		}
	} 
}

void f4()
{
	{
		std::lock(safeMap_copy, safeMap); // transactional semantics; note different order of lock acquisition than in f3()
		if (*safeMap_copy.empty())
		{
			*safeMap_copy = *safeMap;
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
	// this is guaranteed to print 132 since increments happen
	// atomically in threads t1 and t2
	cout << *safeInt << '\n'; 

	// thread_safe_ptr<Foo> allows for transparent indirection for member access
	cout << safeFoo->c << '\n'; 

	// thread_safe_ptr<std::map> allows for subscripting as the underlying 
	// shared resource (a std::map) supports the same
	(*safeMap)[1] = 1;
	(*safeMap)[2] = 2; 

	cout << (*safeMap)[1] << '\n';
	cout << (*safeMap)[2] << '\n'; 

	std::thread t3(f3);
	std::thread t4(f4);

	t3.join();
	t4.join();

	// safeMap_copy got populated in a thread safe manner 
	// in either thread t3 or thread t4
	cout << (*safeMap_copy)[1] << '\n'; 
	cout << (*safeMap_copy)[2] << '\n'; 

	// thread_safe_ptr<std::string> allows for comparisons
	// as the underlying shared resource (std::string) support the same
	// std::lock used here for transactional semantics
	{
		std::lock lk(safeStr1, safeStr2); 
		cout << boolalpha << (*safeStr1 > *safeStr2) << '\n';
		cout << boolalpha << (*safeStr1 != *safeStr2) << '\n'; 
	} 

	return 0; 
} 

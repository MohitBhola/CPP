#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>

using namespace std;

auto& GetResourceReleaseFunctions()
{
    static std::vector<std::function<void()>> resourceReleaseFunctions;
    return resourceReleaseFunctions;
}

template <typename T>
class Handle
{    
	template <typename U>
	static std::unordered_map<uintptr_t, U> resources;

public:

    Handle() = default;
        
	template <typename... Ts>	
	void reset(Ts&&... ts) 
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    
		if (resources<T>.find(thisPtr) == resources<T>.end())
		{
			resources<T>.emplace(std::piecewise_construct, std::forward_as_tuple(thisPtr), std::forward_as_tuple(ts...));
			
			GetResourceReleaseFunctions().push_back([=]()
			{
			    // we *reset* by assigning a default constructed object to the live object
			    if (resources<T>.find(thisPtr) != resources<T>.end())
			    {
			        cout << "Resource Release1.1" << '\n';
			        
			        resources<T>[thisPtr] = T();
				    (void)resources<T>.erase(thisPtr);
			    }
			});
		}
		else
		{
			resources<T>.erase(thisPtr);
			resources<T>.emplace(std::piecewise_construct, std::forward_as_tuple(thisPtr), std::forward_as_tuple(ts...));
		}
	}
		
	template <typename U>
	Handle& operator=(U&& u)
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    
		if (resources<T>.find(thisPtr) == resources<T>.end())
		{
			// this will work only if u could be used to create a T
			resources<T>.insert(std::make_pair(thisPtr, std::forward<U>(u)));
			
			GetResourceReleaseFunctions().push_back([=]()
			{
			    // we *reset* by assigning a default constructed object to the live object
			    if (resources<T>.find(thisPtr) != resources<T>.end())
			    {
			        cout << "Resource Release1.2" << '\n';
			        
			        resources<T>[thisPtr] = T();
				    (void)resources<T>.erase(thisPtr);
			    }
			});
		}
		else
		{
			resources<T>.erase(thisPtr);
			resources<T>.insert(std::make_pair(thisPtr, std::forward<U>(u)));
		}
		
		return *this;
	}
	
	T const& get() const
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    return resources<T>.at(thisPtr);
	}
	
	T& get() 
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    return resources<T>.at(thisPtr);
	}
	
	operator bool() const
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    return resources<T>.find(thisPtr) != resources<T>.end();
	}
};

// specialized for pointers
template <typename T>
class Handle<T*>
{    	
	template <typename U>
	static std::unordered_map<uintptr_t, U*> resourcesToBeDelete;
	
public:

    Handle() = default;
        		
	void reset(T* p) 
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    
		if (resourcesToBeDelete<T>.find(thisPtr) == resourcesToBeDelete<T>.end())
		{
			resourcesToBeDelete<T>.insert(std::make_pair(thisPtr, p));
			
			GetResourceReleaseFunctions().push_back([=]()
			{
			    // we *reset* by assigning a default constructed object to the live object
			    if (resourcesToBeDelete<T>.find(thisPtr) != resourcesToBeDelete<T>.end())
			    {
			        cout << "Resource Release2.1" << '\n';
			        
			        delete resourcesToBeDelete<T>[thisPtr];
				    (void)resourcesToBeDelete<T>.erase(thisPtr);
			    }
			});
		}
		else
		{
			delete resourcesToBeDelete<T>[thisPtr];
			(void)resourcesToBeDelete<T>.erase(thisPtr);
			resourcesToBeDelete<T>.insert(std::make_pair(thisPtr, p));
		}
	}
			
	T const* operator->() const
	{
	    return const_cast<T const*>(const_cast<Handle<T*>>(this)->operator->());
	}
	
	T* operator->() 
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    
	    if (resourcesToBeDelete<T>.find(thisPtr) != resourcesToBeDelete<T>.end())
	    {
		    return resourcesToBeDelete<T>.at(thisPtr);    
	    }
	  	    
	    return nullptr;
	}
	
	operator bool() const
	{
	    static auto thisPtr = reinterpret_cast<uintptr_t>(this);
	    return resourcesToBeDelete<T>.find(thisPtr) != resourcesToBeDelete<T>.end();
	}
};

template <typename T>
template <typename U>
std::unordered_map<uintptr_t, U> Handle<T>::resources;

template <typename T>
template <typename U>
std::unordered_map<uintptr_t, U*> Handle<T*>::resourcesToBeDelete;

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
    Handle<shared_ptr<Foo>> fooHandle{};
    
    if (!fooHandle)
    {
        fooHandle.reset(new Foo(43));
    }
    
    fooHandle.get()->print();
    fooHandle.get().reset(new Foo(101));
    fooHandle.get()->print();
    
    Handle<Foo*> fooPtrHandle{};
    
    if (!fooPtrHandle)
    {
        fooPtrHandle.reset(new Foo(42));
    }
    
    fooPtrHandle->print();
    
    for (auto const& func : GetResourceReleaseFunctions())
    {
        func();
    }
        
	return 0;
}

/*
43                                                                                                                                                                  
Foo dtor                                                                                                                                                            
101                                                                                                                                                                 
42                                                                                                                                                                  
Resource Release1.1                                                                                                                                                 
Foo dtor                                                                                                                                                            
Resource Release2.1                                                                                                                                                 
Foo dtor 
*/

#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

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
    
    std::shared_ptr<std::recursive_mutex> mMutex {std::make_shared<std::recursive_mutex>()};
    
public:

    Handle() = default;
        
    template <typename... Ts>   
    void reset(Ts&&... ts) 
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
        static auto thisPtr = reinterpret_cast<uintptr_t>(this);
        
        if (resources<T>.find(thisPtr) == resources<T>.end())
        {
            resources<T>.emplace(std::piecewise_construct, std::forward_as_tuple(thisPtr), std::forward_as_tuple(ts...));
            
            GetResourceReleaseFunctions().push_back([=]()
            {
                std::lock_guard<std::recursive_mutex> guard(*mMutex);
                
                if (resources<T>.find(thisPtr) != resources<T>.end())
                {
                    cout << "Resource Release1.1" << '\n';
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
        
    // this will work only if u could be used to create a T 
    template <typename U>
    Handle& operator=(U&& u)
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
        static auto thisPtr = reinterpret_cast<uintptr_t>(this);
        
        if (resources<T>.find(thisPtr) == resources<T>.end())
        {
            resources<T>.insert(std::make_pair(thisPtr, std::forward<U>(u)));
            
            GetResourceReleaseFunctions().push_back([=]()
            {
                std::lock_guard<std::recursive_mutex> guard(*mMutex);
                
                if (resources<T>.find(thisPtr) != resources<T>.end())
                {
                    cout << "Resource Release1.2" << '\n';
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
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
        static auto thisPtr = reinterpret_cast<uintptr_t>(this);
        return resources<T>.at(thisPtr);
    }
    
    T& get() 
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
        static auto thisPtr = reinterpret_cast<uintptr_t>(this);
        return resources<T>.at(thisPtr);
    }
    
    operator bool() const
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
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
    
    std::shared_ptr<std::recursive_mutex> mMutex {std::make_shared<std::recursive_mutex>()};
    
public:

    Handle() = default;
                
    void reset(T* p) 
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
         
        static auto thisPtr = reinterpret_cast<uintptr_t>(this);
        
        if (resourcesToBeDelete<T>.find(thisPtr) == resourcesToBeDelete<T>.end())
        {
            resourcesToBeDelete<T>.insert(std::make_pair(thisPtr, p));
            
            GetResourceReleaseFunctions().push_back([=]()
            {
                std::lock_guard<std::recursive_mutex> guard(*mMutex);
                
                if (resourcesToBeDelete<T>.find(thisPtr) != resourcesToBeDelete<T>.end())
                {
                    cout << "Resource Release2.1" << '\n';
                    
                    // in the primary template, it sufficed to just erase the entry from the map
                    // but in the specialization for pointers, we need to free the underlying 
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
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        return const_cast<T const*>(const_cast<Handle<T*>>(this)->operator->());
    }
    
    T* operator->() 
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
        static auto thisPtr = reinterpret_cast<uintptr_t>(this);
        
        if (resourcesToBeDelete<T>.find(thisPtr) != resourcesToBeDelete<T>.end())
        {
            return resourcesToBeDelete<T>.at(thisPtr);    
        }
            
        return nullptr;
    }
    
    operator bool() const
    {
        std::lock_guard<std::recursive_mutex> guard(*mMutex);
        
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

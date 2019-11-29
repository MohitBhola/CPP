#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

template <typename...>
using VoidT = void;

template <typename T, typename = VoidT<>>
struct HasReset : std::false_type
{};

template <typename T>
struct HasReset<T, VoidT<decltype(std::declval<T>().reset())>> : std::true_type
{};

template <typename T, typename = VoidT<>>
struct HasClear : std::false_type
{};

template <typename T>
struct HasClear<T, VoidT<decltype(std::declval<T>().Clear())>> : std::true_type
{};

class ResourceManager
{
private:

    ResourceManager() = default;

    template <typename T>
    static std::unordered_map<ResourceManager const*, std::vector<T>> resourcesToBeReset{};
    
    template <typename T>
    static std::unordered_map<ResourceManager const*, std::vector<T>> resourcesToBeCleared{};
    
    template <typename T>
    static std::unordered_map<ResourceManager const*, std::vector<T>> resourcesToBeDeleted{};
    
    template <typename T, typename Deleter>
    static std::unordered_map<ResourceManager const*, std::vector<std::pair<T, Deleter>>> resourcesToBeDeletedViaDeleter{};

    std::vector<std::function<void(ResourceManager const*)>> resourceReleaseFunctions{};

public:

    template <typename T>
    std::enable_if_t<HasReset<T>::value && !HasClear<T>::value> registerResource(std::reference_wrapper<T> resource)
    {
        if (resourcesToBeReset<T>.find(this) == resourcesToBeReset<T>.end())
        {
            resourceReleaseFunctions.push_back([](ResourceManager const* resourceManager)
            {
                for (auto& resource : resourcesToBeReset<T>[resourceManager])
                {
                    resource.reset();
                }
                
                resourcesToBeReset<T>.erase(resourceManager);
            });
        }

        resourcesToBeReset<T>[this].push_back(resource);
    }
    
    template <typename T>
    std::enable_if_t<!HasReset<T>::value && HasClear<T>::value> registerResource(std::reference_wrapper<T> resource)
    {
        if (resourcesToBeCleared<T>.find(this) == resourcesToBeCleared<T>.end())
        {
            resourceReleaseFunctions.push_back([](ResourceManager const* resourceManager)
            {
                for (auto& resource : resourcesToBeCleared<T>[resourceManager])
                {
                    resource.Clear();
                }

                resourcesToBeCleared<T>.erase(resourceManager);
            });
        }

        resourcesToBeCleared<T>[this].push_back(resource);
    }
    
    template <typename T>
    std::enable_if_t<!HasReset<T>::value && !HasClear<T>::value> registerResource(std::reference_wrapper<T> resource)
    {
        if (resourcesToBeDeleted<T>.find(this) == resourcesToBeDeleted<T>.end())
        {
            resourceReleaseFunctions.push_back([](ResourceManager const* resourceManager)
            {
                for (auto& resource : resourcesToBeDeleted<T>[resourceManager])
                {
                    delete resource;
                }
                
                resourcesToBeDeleted<T>.erase(resourceManager);
            });
        }

        resourcesToBeDeleted<T>[this].push_back(resource);
    }
    
    template <typename T, typename Deleter>
    std::enable_if_t<!HasReset<T>::value && !HasClear<T>::value> registerResource(std::reference_wrapper<T> resource, Deleter&& deleter)
    {
        if (resourcesToBeDeletedViaDeleter<T, Deleter>.find(this) == resourcesToBeDeletedViaDeleter<T, Deleter>.end())
        {
            resourceReleaseFunctions.push_back([](ResourceManager const* resourceManager)
            {
                for (auto& pair : resourcesToBeDeletedViaDeleter<T, Deleter>[resourceManager])
                {
                    auto& resource = pair.first;
                    auto& deleter = pair.second;
                    
                    deleter(resource);
                }
                
                resourcesToBeDeletedViaDeleter<T, Deleter>.erase(resourceManager);
            });
        }

        resourcesToBeDeletedViaDeleter<T, Deleter>[this].push_back(std::make_pair(resource, std::forward<Deleter>(deleter)));
    }

    static ResourceManager& GetResourceManager()
    {
        static ResourceManager rm;
        return rm;
    }

    ResourceManager(ResourceManager const&) = delete;
    ResourceManager(ResourceManager&&) = delete;

    void Flush()
    {
        for (auto const& resourceReleaseFunction : resourceReleaseFunctions)
        {
            resourceReleaseFunction(this);
        }
    }
    
    ~ResourceManager() noexcept = default;
};

template<typename T>
std::unordered_map<ResourceManager const*, std::vector<T>> ResourceManager::resourcesToBeReset;

template<typename T>
std::unordered_map<ResourceManager const*, std::vector<T>> ResourceManager::resourcesToBeCleared;

template<typename T>
std::unordered_map<ResourceManager const*, std::vector<T>> ResourceManager::resourcesToBeDeleted;

template<typename T, typename Deleter>
std::unordered_map<ResourceManager const*, std::vector<std::pair<T, Deleter>>> ResourceManager::resourcesToBeDeletedViaDeleter;

struct Foo
{
    ~Foo()
    {
        std::cout << "Foo getting destroyed" << '\n';
    }
};

struct Bar
{
    ~Bar()
    {
        std::cout << "Bar getting destroyed" << '\n';
    }
};

struct Baz
{
    ~Baz()
    {
        std::cout << "Baz getting destroyed" << '\n';
    }
    
    void Clear()
    {
        std::cout << "Baz::Clear called" << '\n';
    }
};

struct ABC
{
    ~ABC() noexcept
    {
        std::cout << "ABC getting destroyed" << '\n';
    }
};

int main()
{
    auto pFoo = std::make_shared<Foo>();
    auto pBar = std::make_shared<Bar>();
    Baz aBaz{};
    
    ResourceManager::GetResourceManager().registerResource(std::ref(pFoo));
    ResourceManager::GetResourceManager().registerResource(std::ref(pBar));   
    
    ResourceManager::GetResourceManager().registerResource(std::ref(aBaz));   
    
    auto abc1 = new ABC();
    ResourceManager::GetResourceManager().registerResource(std::ref(abc1));
    
    auto abc2 = new ABC();
    ResourceManager::GetResourceManager().registerResource(std::ref(abc2), [](auto const* p) {
        std::cout << "This ABC instance is getting destroyed via a Deleter" << '\n';
        delete p;});   
    
    ResourceManager::GetResourceManager().Flush();
    
    std::cout << pFoo.use_count() << '\n';
    std::cout << pBar.use_count() << '\n';
    
    return 0;
}

/*
Baz::Clear called
Baz getting destroyed
ABC getting destroyed
This ABC instance is getting destroyed via a Deleter
ABC getting destroyed
1
1
Baz getting destroyed
Bar getting destroyed
Foo getting destroyed
*/

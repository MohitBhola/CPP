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

	std::vector<std::function<void(ResourceManager const* rm)>> resourceReleaseFunctions{};

public:

	template <typename T>
	std::enable_if_t<HasReset<T>::value> registerResource(std::reference_wrapper<T> t)
	{
		if (resourcesToBeReset<T>.find(this) == resourcesToBeReset<T>.end())
		{
			resourceReleaseFunctions.push_back([](ResourceManager const* rm)
            {
                for (auto& resource : resourcesToBeReset<T>[rm])
                {
                    resource.reset();
                }

                resourcesToBeReset<T>.erase(rm);
            });
		}

		resourcesToBeReset<T>[this].push_back(t);
	}
	
	template <typename T>
	std::enable_if_t<HasClear<T>::value> registerResource(std::reference_wrapper<T> t)
	{
		if (resourcesToBeCleared<T>.find(this) == resourcesToBeCleared<T>.end())
		{
			resourceReleaseFunctions.push_back([](ResourceManager const* rm)
            {
                for (auto& resource : resourcesToBeCleared<T>[rm])
                {
                    resource.Clear();
                }

                resourcesToBeCleared<T>.erase(rm);
            });
		}

		resourcesToBeCleared<T>[this].push_back(t);
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

template<class T>
std::unordered_map<ResourceManager const*, std::vector<T>> ResourceManager::resourcesToBeReset;

template<class T>
std::unordered_map<ResourceManager const*, std::vector<T>> ResourceManager::resourcesToBeCleared;

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

int main()
{
    auto pFoo = std::make_shared<Foo>();
    auto pBar = std::make_shared<Bar>();
    Baz aBaz{};
    
    ResourceManager::GetResourceManager().registerResource(std::ref(pFoo));
    ResourceManager::GetResourceManager().registerResource(std::ref(pBar));   
    
    ResourceManager::GetResourceManager().registerResource(std::ref(aBaz));   
    
    ResourceManager::GetResourceManager().Flush();
    
    std::cout << pFoo.use_count() << '\n';
    std::cout << pBar.use_count() << '\n';
    
    return 0;
}

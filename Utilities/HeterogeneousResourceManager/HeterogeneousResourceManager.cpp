#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

class ResourceManager
{
private:

	ResourceManager() = default;

	template <typename T>
	static std::unordered_map<ResourceManager const*, std::vector<T>> resources{};

	std::vector<std::function<void(ResourceManager const* rm)>> resourceReleaseFunctions{};

public:

	template <typename T>
	void registerResource(T const& t)
	{
		if (resources<T>.find(this) == resources<T>.end())
		{
			resourceReleaseFunctions.push_back([](ResourceManager const* rm){resources<T>.erase(rm);});
		}

		resources<T>[this].push_back(t);
	}

	static ResourceManager& GetResourceManager()
	{
		static ResourceManager rm;
		return rm;
	}

	ResourceManager(ResourceManager const&) = delete;
	ResourceManager(ResourceManager&&) = delete;

    void Clear()
    {
        for (auto const& resourceReleaseFunction : resourceReleaseFunctions)
		{
			resourceReleaseFunction(this);
		}
    }
    
	~ResourceManager()
	{
		Clear();
	}
};

template<class T>
std::unordered_map<ResourceManager const*, std::vector<T>> ResourceManager::resources;

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


int main()
{
    ResourceManager::GetResourceManager().registerResource(std::make_shared<Foo>());
    ResourceManager::GetResourceManager().registerResource(std::make_shared<Bar>());
    
    ResourceManager::GetResourceManager().Clear();
    
    return 0;
}

#include <utility>
#include <tuple>
#include <map>
#include <iostream>

using namespace std;

struct NoCopyNoMove
{
	NoCopyNoMove(int i, int j) : _i(i), _j(j) {}
	~NoCopyNoMove() = default;

	NoCopyNoMove(NoCopyNoMove const&) = delete;
	NoCopyNoMove(NoCopyNoMove&&) = delete;

	NoCopyNoMove& operator=(NoCopyNoMove const&) = delete;
	NoCopyNoMove& operator=(NoCopyNoMove&&) = delete;
	
	friend ostream& operator<<(ostream& os, NoCopyNoMove const& ncnm)
	{
	    cout << "(" << ncnm._i << "," << ncnm._j << ")";
	    return os;
	}
	
private:

	int _i {};
	int _j {};
};

int main()
{
	// this won't compile
	// the NoCopyNoMove temporaries couldn't be copied/moved into the pair being constructed
	//pair<NoCopyNoMove, NoCopyNoMove> aPair{NoCopyNoMove(0,1), NoCopyNoMove(2,3)};

	// we need to create such types *directly* inside the pair
	// need to provide argument packs for the first and second types
	// need to signal separate construction of the first and second types via piecewise_construct tag
	pair<NoCopyNoMove, NoCopyNoMove> aPair{piecewise_construct, forward_as_tuple(0,1), forward_as_tuple(2,3)};
	cout << "First: " << aPair.first << ", " <<  "Second: " << aPair.second << "\n";

	// a pair is directly created inside the map
	// the argument packs for the key and mapped_type directly create them inside that pair
	map<int, NoCopyNoMove> aMap{};
	
	// again, this won't compile, and for similar reasons
	// that is, the NoCopyNoMove temporaries couldn't be copied/moved into the pair being constructed inside the map
	//aMap.emplace(0, NoCopyNoMove(4,5));
	
	// this is right approach
	// that is, signal piecewise construction of the pair (int const, NoCopyNoMove) *directly* inside the map
	aMap.emplace(piecewise_construct, forward_as_tuple(42), forward_as_tuple(0,1));
	
	// observe
	for (auto const& pair : aMap)
	{
	    cout << "Key: " << pair.first << ", " << "Value: " << pair.second << "\n";
	}

	return 0;
}

/*
First: (0,1), Second: (2,3)
Key: 42, Value: (0,1)
*/

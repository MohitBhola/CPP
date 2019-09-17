#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
using namespace std;

decltype(auto) RemoveEvenNumbersLambda()
{
    static auto sLambda = [](auto&& elem){return elem % 2 == 0;};
    return sLambda;
}

// boilerplate to remove elements from sequential or associative containers *on the fly*
template<
    typename Container>
void DoRemoveEvenNumbers(Container&& c)
{
    auto iter = begin(c);
    auto e = end(c);

    while (iter != e)
    {
        if (RemoveEvenNumbersLambda()(*iter))
        {
            // since C++11, almost all standard containers expose (overloaded) member function erase
            // in its simplest form, this function takes an iterator and erases the corresponding element from the underlying container
            // most importantly, it returns a *valid* iterator pointing to the next element in the container
            // this idiom works for both sequential and associative containers
            // prior to C++11, it used to work only for sequential containers and not for associative containers
            iter = c.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

template<
    typename Container>
void DoPrintContainer(Container&& c)
{
    for (auto const& elem : c)
    {
        cout << elem << '\t';
    }

    cout << '\n';
}

int main()
{
    // sequential container
    vector<int> ivec{0,1,2,3,4,5,6,7,8,9};
    DoRemoveEvenNumbers(ivec);
    DoPrintContainer(ivec);

    // associative container
    set<int> iset{0,1,2,3,4,5,6,7,8,9};
    DoRemoveEvenNumbers(iset);
    DoPrintContainer(iset);

    return 0;
}

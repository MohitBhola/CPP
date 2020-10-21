#include <vector>
#include <set>
#include <iostream>

using namespace std;

/*
 * To get P(n), first get P(n-1) and add the sets in it to P(n)
 * Next, add n to each set in P(n-1) and add the resulting sets to P(n) to get the final answer.
 */
vector<set<char>> getPowerSet(vector<char>& cvec, int index)
{
    if (index == -1)
    {
        return vector<set<char>>(1, set<char>());
    }
    
    vector<set<char>> results{};
    int ch = cvec[index];
    
    auto smallerPowerSet = getPowerSet(cvec, index-1);
    results = smallerPowerSet;
    
    for (auto& set : smallerPowerSet)
    {
        set.insert(ch);
        results.push_back(set);
    }
    
    return results;
}

int main()
{
    vector<char> cvec{'a','b','c'};
    
    for (auto const& set : getPowerSet(cvec, static_cast<int>(cvec.size()-1)))
    {
        if (set.empty())
        {
            cout << "{}" << '\n';
        }
        else
        {
            cout << "{ ";
            
            for (char ch : set)
            {
                cout << ch << " ";
            }
            
            cout << "}" << '\n';
        }
    }
    
    return 0;
}

/*
{}
{ a }
{ b }
{ a b }
{ c }
{ a c }
{ b c }
{ a b c }
*/

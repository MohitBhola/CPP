#include <sstream>
#include <iostream>
#include <iterator>
#include <deque>
#include <initializer_list>
using namespace std;

int main()
{
    deque<int> ideque{};
    
    // back insertion
    istream_iterator<int> it_cin{cin};
    istream_iterator<int> it_cin_end{};
    copy(it_cin, it_cin_end, back_inserter(ideque));
    
    // middle insertion
    using difference_type = typename iterator_traits<decltype(ideque.begin())>::difference_type;
    
    istringstream iss{"123 456 7890"};
    istream_iterator<int> it_iss{iss};
    istream_iterator<int> it_iss_end{};
    copy(it_iss, it_iss_end, inserter(ideque, next(begin(ideque), static_cast<difference_type>((ideque.size() / 2)))));
    
    // front insertion
    initializer_list<int> iil{-1, -2, -3};
    copy(begin(iil), end(iil), front_inserter(ideque));
    
    // print
    for (auto const& num : ideque)
    {
        cout << num << '\t';
    }
    
    cout << '\n';

    return 0;
}



/*
1 2 3 4 5
-3	-2	-1	1	2	123	456	7890	3	4	5	
*/


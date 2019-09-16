#include <vector>
#include <iostream>
using namespace std;

int main()
{
    vector<int> ivec(42, 42);

    cout << "Initial size: " << ivec.size() << '\n';
    cout << "Initial capacity: " << ivec.capacity() << '\n';

    // the standard idiom to clear off both size and capacity
    // create a temporary vector whose both size and capacity are 0 (or maybe the capacity is some implementation defined minimum)
    // then swap the *internals* of this vector with the target vector to *shrink*
	// note that the temporary gets destroyed at the end of this statement!
    vector<int>().swap(ivec);

    cout << "Initial size: " << ivec.size() << '\n';
    cout << "Initial capacity: " << ivec.capacity() << '\n';

    return 0;
}

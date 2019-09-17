// a simple, portable method to have two *different* types
// who could be distinguished on the basis of their size at compile time
template<
    typename T>
class larger_than
{
    T body_[2];
};    

// these are typically used to indicate success/failure of a compile time check in real programs
using no_type = char;
using yes_type = larger_than<no_type>;

int main()
{
	static_assert(sizeof(yes_type) >= sizeof(no_type));
	
	return 0;
}

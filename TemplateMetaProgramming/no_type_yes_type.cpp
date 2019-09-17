template<
    typename T>
class larger_than
{
    T body_[2];
};    

using no_type = char;
using yes_type = larger_than<no_type>;

int main()
{
	static_assert(sizeof(yes_type) >= sizeof(no_type));
	
	return 0;
}

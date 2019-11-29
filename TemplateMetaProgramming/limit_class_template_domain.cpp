// a class template can be seen as a metafunction that maps a tuple of parameters to a concrete class
// *explicit specialization* can limit the domain of the class template
// that is, we can have a general template and then some specializations, and each of these may or may not have a body!

// for example, the following template can only be instantiated for int and double
template <typename T>
struct Foo;

template <>
struct Foo<int>
{};

template <>
struct Foo<double>
{};

template <typename T>
void supressUnusedVariableWarning(T const& t)
{}

int main()
{
	Foo<int> iFoo{}; // OK
	supressUnusedVariableWarning(iFoo);
	
	Foo<double> dFoo{}; //OK
	supressUnusedVariableWarning(dFoo);
	
	//Foo<char> cFoo{}; // NOK

	return 0;
}



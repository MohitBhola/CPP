/*
 * An implicit copy ctor of the Derived class would always invoke the implicit copy ctor of the Base class.
 * If one isn't present in the Base class, it would be synthesized.
 *
 * However, if an *explicitly* provided copy ctor of the Derived class invokes the copy ctor in the Base class,
 * a universal ctor in the Base class (if present) would then participate in overload resolution with the
 * implicitly synthesized copy ctor in the Base class, and thus may be preferred if it provides a better match
 *
 * For example, the following program results in Stack Overflow.
 */

struct Base
{
	Base() = default;
	Base(Base const& other) = default;

	// every time, T is deduced as Derived
	// this recurses back to the Derived class copy ctor, and gets called again, ad nauseam !!!
	// thus resulting in StackOverflow
	// Note that this universal ctor provides an exact match for the Derived class object, and thus gets precedence
  // over other ctors (implicit or explicit) that feature a parameter related to the the Base class

	template<
		typename T>
	Base(T x)
	{}
};

struct Derived : Base
{
	Derived() = default;

	Derived(Derived const& d) : Base(d)
	{}
};

int main()
{
	Derived d1;
	Derived d2 = d1;

	return 0;
}

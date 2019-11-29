// a universal copy constructor is never a copy constructor; the presence of the former doesn't suppresses the implicit declaration of the latter
// universal constructors participate in overload resolution with other constructors, including copy constructors; they may be preferred over others if they provide a better match

struct base
{
    base() = default;
    
    template <typename T>
    base(T x)
    {}
};

struct derived : base
{
    derived() = default;
    
    // an implicit copy constructor must invoke the copy constructor of the base class; it will never call a universal constructor
    // however, if a user provided copy constructor for the derived class tries to invoke its base class counterpart, it can go horribly wrong in the presence of universal constructors
    // here, the universal constructor of the base class gets called as it provides an exact match, but since it takes its argument by value, the call becomes recursive resulting in StackOverflow
    derived(derived const& that) : base(that)
    {}
};

int main()
{
    derived d1{};
    derived d2 = d1;
    
    return 0;
}

/*
main.cpp: In function 'int main()':
main.cpp:27:13: warning: variable 'd2' set but not used [-Wunused-but-set-variable]
   27 |     derived d2 = d1;
      |             ^~
bash: line 7: 17327 Segmentation fault      (core dumped) ./a.out
*/

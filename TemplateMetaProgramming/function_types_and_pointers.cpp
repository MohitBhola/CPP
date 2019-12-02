// a function decays to a function pointer
// of course, a function pointer can be constructed
// BUT, a function type cannot be constructed

template <typename T>
struct X
{
    T m_t;
    X(T t) : m_t(t) {}
};

double area(double radius)
{
    return 3.14 * radius * radius;
}

template <typename T>
X<T> identify_by_val(T val)
{
    return X<T>(val);
}

template <typename T>
X<T> identify_by_ref(T const& val)
{
    return X<T>(val);
}

int main()
{
    // in examples below, definitions of x1 and x4 lead to instantiation of the struct X
    // with a function type (double(double)), which cannot be copy constructed in the m_t data member
    // thus, they lead to compilation errors
    
    //X<double(double)> x1(area);
    X<double(*)(double)> x2(area);
    
    auto x3 = identify_by_val(area);
    //auto x4 = identify_by_ref(area);
    
    return 0;    
}



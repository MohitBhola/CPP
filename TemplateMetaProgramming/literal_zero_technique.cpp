class dummy
{};

using literal_zero_t = int dummy::*;

void Foo(literal_zero_t zero)
{}

int main()
{
    Foo(0);
    //Foo(42);
    
    return 0;
}

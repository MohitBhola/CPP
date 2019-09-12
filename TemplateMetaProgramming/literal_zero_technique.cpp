class dummy
{};

using literal_zero_t = int dummy::*;

void Foo(literal_zero_t zero)
{}

int main()
{
    literal_zero_t zero{};
    Foo(zero);
    
    return 0;
}

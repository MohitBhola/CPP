// what if you want to be able to pass 0 but no other integer literal to a function?
// literal zero technique to the rescue!
// define a literal zero type as a pointer to an int member of a dummy class
// such a type could only be instantiated with a literal 0 :)

class dummy
{};

using literal_zero_t = int dummy::*;

void Foo(literal_zero_t zero)
{}

int main()
{
    Foo(0);  // OK
    Foo(42); // NOK
    
    return 0;
}

/*
main.cpp: In function 'int main()':
main.cpp:17:9: error: cannot convert 'int' to 'literal_zero_t' {aka 'int dummy::*'}
   17 |     Foo(42); // NOK
      |         ^~
      |         |
      |         int
main.cpp:11:25: note:   initializing argument 1 of 'void Foo(literal_zero_t)'
   11 | void Foo(literal_zero_t zero)
      |          ~~~~~~~~~~~~~~~^~~~
*/

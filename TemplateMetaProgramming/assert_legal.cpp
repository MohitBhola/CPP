#include <vector>
using namespace std;

#define ASSERT_LEGAL(statement)     sizeof(statement, 0)
#define CONST_REF_TO(T)             *(static_cast<T const*>(0))
#define REF_TO(T)                   *(static_cast<T*>(0))

template<
    typename obj_t,
    typename iter_t>
class assert_iterator
{
    enum class Assertions
    {
        construction = ASSERT_LEGAL(obj_t(*CONST_REF_TO(iter_t))),
        assignment = ASSERT_LEGAL(REF_TO(obj_t) = *(CONST_REF_TO(iter_t))),
        preincr = ASSERT_LEGAL(++(REF_TO(iter_t))),
        postincr = ASSERT_LEGAL((REF_TO(iter_t))++)
    };
};    

int main()
{
    vector<int> ivec{0};
    assert_iterator<int, decltype(ivec.begin())> ai1{};
    
    assert_iterator<int, double*> ai2{};
    
    return 0;
}

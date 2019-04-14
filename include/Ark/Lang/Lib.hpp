#ifndef ark_lib
#define ark_lib

#include <Ark/Lang/Node.hpp>
#include <Ark/Lang/Environment.hpp>

#define FUNCTION(name) Node name(const Nodes& n);

namespace Ark
{
    namespace Lang
    {
        FUNCTION(add)  // +
        FUNCTION(sub)  // -
        FUNCTION(mul)  // *
        FUNCTION(div)  // /

        FUNCTION(gt)  // >
        FUNCTION(lt)  // <
        FUNCTION(le)  // <=
        FUNCTION(ge)  // >=
        FUNCTION(neq)  // !=
        FUNCTION(eq)  // =

        FUNCTION(len)  // len 1
        FUNCTION(empty)  // empty? 1
        FUNCTION(car)  // car 1
        FUNCTION(cdr)  // cdr +
        FUNCTION(append)  // append +
        FUNCTION(cons)  // cons +
        FUNCTION(list)  // list +
        FUNCTION(isnil)  // nil? 1

        FUNCTION(print)  // print +
        FUNCTION(assert_)  // assert 2

        void registerLib(Environment& env);
    }
}

#undef FUNCTION

#endif  // ark_lib

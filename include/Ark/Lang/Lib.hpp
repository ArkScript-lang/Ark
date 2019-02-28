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
//        FUNCTION(mul)  // *
//        FUNCTION(div)  // /

//        FUNCTION(gt)  // >
        FUNCTION(lt)  // <
//        FUNCTION(le)  // <=
//        FUNCTION(ge)  // >=
        FUNCTION(neq)  // !=
        FUNCTION(eq)  // =

//        FUNCTION(len)  // len
//        FUNCTION(empty)  // empty
//        FUNCTION(car)  // car
//        FUNCTION(cdr)  // cdr
//        FUNCTION(append)  // append
//        FUNCTION(cons)  // cons
//        FUNCTION(list)  // list
//        FUNCTION(isnil)  // nil?
        FUNCTION(print)  // print

        void registerLib(Environment& env);
    }
}

#undef FUNCTION

#endif  // ark_lib

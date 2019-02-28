#ifndef ark_lib
#define ark_lib

#include <Ark/Lang/Node.hpp>
#include <Ark/Lang/Environment.hpp>

#define FUNCTION(name) Node name(const Nodes& n);
#define CHECK_ARGUMENTS(expected, args, name) if (args.size() < expected || args.size() > expected) { Ark::Log::error("[Argument Error] '" + #name + "' needs " + #expected + " arguments"); exit(1); }
#define AT_LEAST_ARGUMENTS(expected, args, name) if (expected > args.size()) { Ark::Log::error("[Argument Error] '" + #name + "' needs at least " + #expected + " arguments"); exit(1); }

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

//        FUNCTION(len)  // len
//        FUNCTION(empty)  // empty
//        FUNCTION(car)  // car
//        FUNCTION(cdr)  // cdr
//        FUNCTION(append)  // append
//        FUNCTION(cons)  // cons
//        FUNCTION(list)  // list
//        FUNCTION(isnil)  // nil?
        FUNCTION(print)  // print
        FUNCTION(assert)  // assert

        void registerLib(Environment& env);
    }
}

#undef FUNCTION

#endif  // ark_lib

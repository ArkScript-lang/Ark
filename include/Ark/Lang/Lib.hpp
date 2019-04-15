#ifndef ark_lib
#define ark_lib

#include <vector>
#include <string>

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
        FUNCTION(firstof)  // firstof 1
        FUNCTION(tailof)  // tailof +
        FUNCTION(append)  // append +
        FUNCTION(concat)  // concat +
        FUNCTION(list)  // list +
        FUNCTION(isnil)  // nil? 1

        FUNCTION(print)  // print +
        FUNCTION(assert_)  // assert 2

        void registerLib(Environment& env);

        extern const std::vector<std::string> builtins;
    }
}

#undef FUNCTION

#endif  // ark_lib

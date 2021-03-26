#ifndef ARK_COMPILER_MAKENODEBASEDERROR_INL
#define ARK_COMPILER_MAKENODEBASEDERROR_INL

#include <sstream>
#include <string>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    std::string makeNodeBasedErrorCtx(const std::string& message, const Ark::internal::Node& node);
}

#endif

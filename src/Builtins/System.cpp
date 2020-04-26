#include <Ark/Builtins/Builtins.hpp>

#include <cstdlib>
#include <Ark/Constants.hpp>

#include <Ark/Builtins/BuiltinsErrors.inl>
#define Builtins_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::Builtins::System
{
    Builtins_Function(system_)
    {
        if (n.size() != 1)
            throw std::runtime_error(SYS_SYS_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(SYS_SYS_TE0);
        
        #if ARK_ENABLE_SYSTEM != 0
            int output = std::system(n[0].string().c_str());
            return Value(output);
        #endif  // ARK_ENABLE_SYSTEM
        
        return nil;
    }
}
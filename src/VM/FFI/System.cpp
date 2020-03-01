#include <Ark/VM/FFI.hpp>

#include <cstdlib>
#include <Ark/Constants.hpp>

#define FFI_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::FFI::System
{
    FFI_Function(system_)
    {
        if (n.size() != 1)
            throw std::runtime_error("system can take only one argument, a command");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument of system must be of type String");
        
        #if ARK_ENABLE_SYSTEM != 0
            int output = std::system(n[0].string().c_str());
            return Value(output);
        #endif  // ARK_ENABLE_SYSTEM
        
        return nil;
    }
}
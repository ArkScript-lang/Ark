#include <Ark/VM/FFI.hpp>

#include <cstdlib>
#include <Ark/Constants.hpp>

namespace Ark::internal::FFI
{
    FFI_Function(system_)
    {
        if (n.size() != 1)
            throw std::runtime_error("system can take only one argument, a command");
        if (n[0].valueType() != ValueType::String)
            throw std::runtime_error("Argument of system must be of type String");
        
        #if ARK_ENABLE_SYSTEM != 0
            std::system(n[0].string().c_str());
        #endif  // ARK_ENABLE_SYSTEM
        
        return nil;
    }
}
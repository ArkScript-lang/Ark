#ifndef ark_vm_state
#define ark_vm_state

#include <string>
#include <vector>
#include <cinttypes>
#include <unordered_map>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>

namespace Ark
{
    class State
    {
    public:
        State(const std::string& libdir="", const std::string& filename="FILE");

        template <bool D> friend class VM_t;
    
    private:
        bytecode_t m_bytecode;
        std::string m_libdir;
        std::string m_filename;

        // related to the bytecode
        std::vector<std::string> m_symbols;
        std::vector<internal::Value> m_constants;
        std::vector<std::string> m_plugins;
        std::vector<internal::SharedLibrary> m_shared_lib_objects;
        std::vector<bytecode_t> m_pages;

        // related to the execution
        std::unordered_map<std::string, internal::Value::ProcType> m_binded_functions;
    };
}

#endif
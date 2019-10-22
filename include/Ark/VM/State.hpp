#ifndef ark_vm_state
#define ark_vm_state

#include <string>
#include <vector>
#include <cinttypes>
#include <unordered_map>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/VM/Plugin.hpp>
#include <Ark/Log.hpp>

namespace Ark
{
    class State
    {
    public:
        State(const std::string& libdir="", const std::string& filename="FILE", uint16_t options=DefaultFeatures);

        // for already compiled ArkScript files
        bool feed(const std::string& bytecode_filename);
        bool feed(const bytecode_t& bytecode);

        // to compile file *only* if needed and use the resulting bytecode
        bool doFile(const std::string& filename);
        // compile string and store resulting bytecode in m_bytecode
        bool doString(const std::string& code);

        void loadFunction(const std::string& name, internal::Value::ProcType function);
        void setDebug(bool value);

        template <bool D> friend class VM_t;
    
    private:
        void configure();

        inline void throwStateError(const std::string& message)
        {
            throw std::runtime_error("StateError: " + message);
        }

        bool m_debug;

        bytecode_t m_bytecode;
        std::string m_libdir;
        std::string m_filename;
        uint16_t m_options;

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
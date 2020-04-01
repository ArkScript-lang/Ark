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
    /**
     * @brief Ark state to handle the dirty job of loading and compiling ArkScript code
     * 
     */
    class State
    {
    public:
        /**
         * @brief Construct a new State object
         * 
         * @param libdir the path to the standard library, defaults to "?" which means: search in environment variables
         * @param options the options for the virtual machine, compiler, and parser
         */
        State(const std::string& libdir="?", uint16_t options=DefaultFeatures);

        /**
         * @brief Feed the state by giving it the path to an existing bytecode file
         * 
         * @param bytecode_filename 
         * @return true on success
         * @return false on failure
         */
        bool feed(const std::string& bytecode_filename);

        /**
         * @brief Feed the state with ArkScript bytecode
         * 
         * @param bytecode 
         * @return true on success
         * @return false on failure
         */
        bool feed(const bytecode_t& bytecode);

        /**
         * @brief Compile a file *only* if needed, and use the resulting bytecode
         * 
         * @param filename path to an ArkScript code file
         * @return true on success
         * @return false on failure
         */
        bool doFile(const std::string& filename);

        /**
         * @brief Compile a string (representing ArkScript code) and store resulting bytecode in m_bytecode
         * 
         * @param code the ArkScript code
         * @return true on success
         * @return false on failure
         */
        bool doString(const std::string& code);

        /**
         * @brief Register a function in the virtual machine
         * 
         * @param name the name of the function in ArkScript
         * @param function the code of the function
         */
        void loadFunction(const std::string& name, internal::Value::ProcType function);

        /**
         * @brief Set the debug level
         * 
         * @param level between 0 (nothing) and 3 (maximum verbosity)
         */
        void setDebug(unsigned level);

        template <bool D> friend class VM_t;

    private:
        /**
         * @brief Called to configure the state (set the bytecode, debug level, call the compiler...)
         * 
         */
        void configure();

        inline void throwStateError(const std::string& message)
        {
            throw std::runtime_error("StateError: " + message);
        }

        unsigned m_debug_level;

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
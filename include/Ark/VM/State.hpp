/**
 * @file State.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief State used by the virtual machine: it loads the bytecode, can compile it if needed, load C++ functions...
 * @version 0.3
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_VM_STATE_HPP
#define ARK_VM_STATE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/Common.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    /**
     * @brief Ark state to handle the dirty job of loading and compiling ArkScript code
     *
     */
    class ARK_API State
    {
    public:
        /**
         * @brief Construct a new State object
         *
         * @param libenv a list of search paths for the std library
         */
        explicit State(const std::vector<std::filesystem::path>& libenv = {}) noexcept;

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
         * @brief Compile a file, and use the resulting bytecode
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
        void loadFunction(const std::string& name, Value::ProcType function) noexcept;

        /**
         * @brief Set the script arguments in sys:args
         *
         * @param args
         */
        void setArgs(const std::vector<std::string>& args) noexcept;

        /**
         * @brief Set the debug level
         *
         * @param level between 0 (nothing) and 3 (maximum verbosity)
         */
        void setDebug(unsigned level) noexcept;

        /**
         * @brief Set the std search paths
         *
         * @param libenv the list of std search paths to set
         */
        void setLibDirs(const std::vector<std::filesystem::path>& libenv) noexcept;

        /**
         * @brief Reset State (all member variables related to execution)
         *
         */
        void reset() noexcept;

        friend class VM;
        friend class internal::Closure;
        friend class Repl;

    private:
        /**
         * @brief Called to configure the state (set the bytecode, debug level, call the compiler...)
         *
         * @param bcr reference to a pre-fed bytecode reader
         */
        void configure(const BytecodeReader& bcr);

        /**
         * @brief Reads and compiles code of file
         *
         * @param file the path of file code to compile
         * @param output set path of .arkc file
         * @return true on success
         * @return false on failure and raise an exception
         */
        bool compile(const std::string& file, const std::string& output) const;

        static void throwStateError(const std::string& message)
        {
            throw Error("StateError: " + message);
        }

        unsigned m_debug_level;

        bytecode_t m_bytecode;
        std::vector<std::filesystem::path> m_libenv;
        std::string m_filename;

        // related to the bytecode
        std::vector<std::string> m_symbols;
        std::vector<Value> m_constants;
        std::vector<bytecode_t> m_pages;

        // related to the execution
        std::unordered_map<std::string, Value> m_binded;
    };
}

#endif

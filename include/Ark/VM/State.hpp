/**
 * @file State.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief State used by the virtual machine: it loads the bytecode, can compile it if needed, load C++ functions...
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2025
 *
 */

#ifndef ARK_VM_STATE_HPP
#define ARK_VM_STATE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <Ark/Constants.hpp>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/Value/Procedure.hpp>
#include <Ark/Compiler/Common.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Compiler/IntermediateRepresentation/InstLoc.hpp>

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
         * @param file_path path to an ArkScript code file
         * @param features compiler features to enable/disable
         * @return true on success
         * @return false on failure
         */
        bool doFile(const std::string& file_path, uint16_t features = DefaultFeatures);

        /**
         * @brief Compile a string (representing ArkScript code) and store resulting bytecode in m_bytecode
         *
         * @param code the ArkScript code
         * @param features compiler features to enable/disable
         * @return true on success
         * @return false on failure
         */
        bool doString(const std::string& code, uint16_t features = DefaultFeatures);

        /**
         * @brief Register a function in the virtual machine
         *
         * @param name the name of the function in ArkScript
         * @param function the code of the function
         */
        void loadFunction(const std::string& name, Procedure::CallbackType&& function) noexcept;

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
         * @param features compiler features to enable/disable
         * @return true on success
         * @return false on failure and raise an exception
         */
        [[nodiscard]] bool compile(const std::string& file, const std::string& output, uint16_t features) const;

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
        std::vector<std::string> m_filenames;
        std::vector<internal::InstLoc> m_inst_locations;
        std::size_t m_max_page_size;
        bytecode_t m_code;

        // related to the execution
        std::unordered_map<std::string, Value> m_binded;  ///< Values binded to the State, to be used by the VM

        /**
         * @brief Get an instruction in a given page, with a given instruction pointer
         *
         * @param pp page pointer
         * @param ip instruction pointer
         * @return uint8_t instruction
         */
        [[nodiscard]] inline constexpr uint8_t inst(const std::size_t pp, const std::size_t ip) const noexcept
        {
            return m_code[pp * m_max_page_size + ip];
        }
    };
}

#endif

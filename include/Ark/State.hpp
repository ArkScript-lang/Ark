/**
 * @file State.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief State used by the virtual machine: it loads the bytecode, can compile it if needed, load C++ functions...
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_VM_STATE_HPP
#define ARK_VM_STATE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <Ark/Constants.hpp>

#include <Ark/VM/Value/Value.hpp>
#include <Ark/VM/Value/Procedure.hpp>
#include <Ark/Compiler/Common.hpp>
#include <Ark/Error/Exceptions.hpp>
#include <Ark/Compiler/IntermediateRepresentation/InstLoc.hpp>

namespace Ark
{
    namespace internal
    {
        class Debugger;
    }

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
         * @param fail_with_exception
         * @return true on success
         * @return false on failure
         */
        bool feed(const std::string& bytecode_filename, bool fail_with_exception = false);

        /**
         * @brief Feed the state with ArkScript bytecode
         *
         * @param bytecode
         * @param fail_with_exception
         * @return true on success
         * @return false on failure
         */
        bool feed(const bytecode_t& bytecode, bool fail_with_exception = false);

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

        /**
         * @brief Used by the debugger to add code to the VM at runtime
         *
         * @param pages
         * @param symbols
         * @param constants
         */
        void extendBytecode(const std::vector<bytecode_t>& pages, const std::vector<std::string>& symbols, const std::vector<Value>& constants);

        [[nodiscard]] inline const bytecode_t& bytecode() const noexcept
        {
            return m_bytecode;
        }

        friend class VM;
        friend class Repl;
        friend class internal::Closure;
        friend class internal::Debugger;

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
        [[nodiscard]] bool compile(const std::string& file, const std::string& output) const;

        static void throwStateError(const std::string& message)
        {
            throw Error("StateError: " + message);
        }

        unsigned m_debug_level;
        uint16_t m_features;

        bytecode_t m_bytecode;
        std::vector<std::filesystem::path> m_libenv;
        std::string m_filename;

        // related to the bytecode
        std::vector<std::string> m_symbols;
        std::vector<Value> m_constants;
        std::vector<std::string> m_filenames;
        std::vector<internal::InstLoc> m_inst_locations;
        std::vector<bytecode_t> m_pages;
        std::size_t m_max_page_size;
        bytecode_t m_code;

        // related to the execution
        std::unordered_map<std::string, Value> m_bound;  ///< Values bound to the State, to be used by the VM

        void addPagesToContiguousBytecode(const std::vector<bytecode_t>& pages, std::size_t start);

        /**
         * @brief Compute the maximum length of the given code pages
         *
         * @param pages
         * @return std::size_t
         */
        static std::size_t maxPageSize(const std::vector<bytecode_t>& pages);

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

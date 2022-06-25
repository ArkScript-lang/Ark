#ifndef ARK_COMPILER_JSONCOMPILER_HPP
#define ARK_COMPILER_JSONCOMPILER_HPP

#include <vector>
#include <string>

#include <Ark/Constants.hpp>
#include <Ark/Platform.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Compiler/AST/Optimizer.hpp>

namespace Ark
{
    class ARK_API JsonCompiler
    {
    public:
        /**
         * @brief Construct a new JsonCompiler object
         *
         * @param debug the debug level
         * @param options the compilers options
         */
        JsonCompiler(unsigned debug, const std::vector<std::string>& libenv, uint16_t options = DefaultFeatures);

        /**
         * @brief Feed the differents variables with information taken from the given source code file
         *
         * @param code the code of the file
         * @param filename the name of the file
         */
        void feed(const std::string& code, const std::string& filename = ARK_NO_NAME_FILE);

        /**
         * @brief Start the compilation
         *
         * @return
         */
        std::string compile();

    private:
        internal::Parser m_parser;
        internal::Optimizer m_optimizer;
        uint16_t m_options;
        unsigned m_debug;  ///< the debug level of the compiler

        /**
         * @brief Compile a single node and return its representation
         *
         * @param node
         * @return const std::string&
         */
        std::string _compile(const internal::Node& node);

        /**
         * @brief Convert a NodeType::List to a JSON list
         *
         * @param node
         * @param start
         * @return std::string
         */
        std::string toJsonList(const internal::Node& node, std::size_t start);
    };
}

#endif

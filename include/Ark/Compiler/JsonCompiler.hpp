#ifndef ARK_COMPILER_JSONCOMPILER_HPP
#define ARK_COMPILER_JSONCOMPILER_HPP

#include <vector>
#include <string>
#include <filesystem>

#include <Ark/Constants.hpp>
#include <Ark/Platform.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/Welder.hpp>

namespace Ark
{
    class ARK_API JsonCompiler final
    {
    public:
        /**
         * @brief Construct a new JsonCompiler object
         *
         * @param debug the debug level
         */
        JsonCompiler(unsigned debug, const std::vector<std::filesystem::path>& libenv);

        /**
         * @brief Feed the different variables with information taken from the given source code file
         *
         * @param filename the name of the file
         */
        void feed(const std::string& filename);

        /**
         * @brief Start the compilation
         *
         * @return
         */
        std::string compile();

    private:
        Welder m_welder;

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

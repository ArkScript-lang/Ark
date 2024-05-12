#ifndef CLI_JSONCOMPILER_HPP
#define CLI_JSONCOMPILER_HPP

#include <vector>
#include <string>
#include <filesystem>

#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/Welder.hpp>

class JsonCompiler final
{
public:
    /**
     * @brief Construct a new JsonCompiler object
     *
     * @param debug the debug level
     * @param lib_env list of path to the directories of the std lib
     */
    JsonCompiler(unsigned debug, const std::vector<std::filesystem::path>& lib_env);

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
    Ark::Welder m_welder;

    /**
     * @brief Compile a single node and return its representation
     *
     * @param node
     * @return const std::string&
     */
    std::string _compile(const Ark::internal::Node& node);

    /**
     * @brief Convert a NodeType::List to a JSON list
     *
     * @param node
     * @param start
     * @return std::string
     */
    std::string toJsonList(const Ark::internal::Node& node, std::size_t start);
};

#endif

#ifndef ARK_FORMATTER_HPP
#define ARK_FORMATTER_HPP

#include <string>

#include <Ark/Compiler/AST/Parser.hpp>

class Formatter final
{
public:
    Formatter(std::string filename, bool dry_run);

    void run();

    const std::string& output() const;

private:
    const std::string m_filename;
    bool m_dry_run;  ///< If true, only prints the formatted file instead of saving it to disk
    Ark::internal::Parser m_parser;
    std::string m_output;

    bool isListStartingWithKeyword(const Ark::internal::Node& node, Ark::internal::Keyword keyword);
    bool isBeginBlock(const Ark::internal::Node& node);
    bool isFuncDef(const Ark::internal::Node& node);
    bool isFuncCall(const Ark::internal::Node& node);

    /**
     * @param node
     * @return true if the node is a String|Number|Symbol|Field
     * @return false
     */
    bool isPlainValue(const Ark::internal::Node& node);

    /**
     * @brief Compute the line on which the deepest right most node of node is at
     * @param node
     * @return
     */
    std::size_t lineOfLastNodeIn(const Ark::internal::Node& node);

    bool should_split_on_newline(const Ark::internal::Node& node);

    /**
     * @brief Handles all node formatting
     * @param node
     * @param indent indentation level, starting at 0, increment by 1
     * @param after_newline when false, do not add prefix
     * @return
     */
    std::string format(const Ark::internal::Node& node, std::size_t indent, bool after_newline);

    std::string formatBlock(const Ark::internal::Node& node, std::size_t indent, bool after_newline);

    std::string formatFunction(const Ark::internal::Node& node, std::size_t indent);
    std::string formatVariable(const Ark::internal::Node& node, std::size_t indent);
    std::string formatCondition(const Ark::internal::Node& node, std::size_t indent, bool is_macro = false);
    std::string formatLoop(const Ark::internal::Node& node, std::size_t indent);
    std::string formatBegin(const Ark::internal::Node& node, std::size_t indent, bool after_newline);
    std::string formatImport(const Ark::internal::Node& node, std::size_t indent);
    std::string formatDel(const Ark::internal::Node& node, std::size_t indent);
    std::string formatCall(const Ark::internal::Node& node, std::size_t indent);
    std::string formatMacro(const Ark::internal::Node& node, std::size_t indent);
};

#endif  // ARK_FORMATTER_HPP
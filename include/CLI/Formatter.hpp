#ifndef ARK_FORMATTER_HPP
#define ARK_FORMATTER_HPP

#include <string>

#include <Ark/Compiler/AST/Parser.hpp>

constexpr struct FormatterConfig
{
    static constexpr std::size_t SpacePerIndent = 2;   ///< Indentation level of each node
    static constexpr std::size_t LongLineLength = 32;  ///< Max number of characters per line segment to consider splitting
} FormatterConfig;

class Formatter final
{
public:
    explicit Formatter(bool dry_run);
    Formatter(std::string filename, bool dry_run);

    /**
     * @brief Read the file and process it. The file isn't modified
     */
    void run();

    /**
     *
     * @param code code to process (bypass reading the file if initialized with a filename)
     */
    void runWithString(const std::string& code);

    [[nodiscard]] const std::string& output() const;

    /**
     *
     * @return true if code has been modified by the formatter
     */
    [[nodiscard]] bool codeModified() const;

private:
    const std::string m_filename;
    bool m_dry_run;  ///< If true, only prints the formatted file instead of saving it to disk
    Ark::internal::Parser m_parser;
    std::string m_output;
    bool m_updated;  ///< True if the original code now difer from the formatted one

    void processAst(const Ark::internal::Node& ast);

    /**
     * @brief Given the original code, produce a warning if comments from it were removed during formatting
     * @param original_code
     * @param filename
     */
    void warnIfCommentsWereRemoved(const std::string& original_code, const std::string& filename);

    /**
     * @brief Check if a given node starts with a given keyword
     * @param node
     * @param keyword
     * @return bool
     */
    [[nodiscard]] static bool isListStartingWithKeyword(const Ark::internal::Node& node, Ark::internal::Keyword keyword);

    /**
     * @brief Check if a node is a begin block
     * @param node
     * @return bool
     */
    [[nodiscard]] static bool isBeginBlock(const Ark::internal::Node& node);

    /**
     * @brief Check if a node is a function definition (fun (args) body)
     * @param node
     * @return bool
     */
    [[nodiscard]] static bool isFuncDef(const Ark::internal::Node& node);

    /**
     * @brief Check if a node is a function call (foo bar egg)
     * @param node
     * @return bool
     */
    [[nodiscard]] static bool isFuncCall(const Ark::internal::Node& node);

    /**
     * @param node
     * @return true if the node is a String|Number|Symbol|Field
     * @return false
     */
    static bool isPlainValue(const Ark::internal::Node& node);

    /**
     * @brief Compute the line on which the deepest right most node of node is at
     * @param node
     * @return
     */
    static std::size_t lineOfLastNodeIn(const Ark::internal::Node& node);

    /**
     * @brief Decide if a node should be split on a newline or not
     * @param node
     * @return bool
     */
    [[nodiscard]] bool shouldSplitOnNewline(const Ark::internal::Node& node);

    /**
     * @brief Compute indentation level
     * @param indent indentation level
     * @return std::string
     */
    static std::string prefix(const std::size_t indent)
    {
        return std::string(indent * FormatterConfig.SpacePerIndent, ' ');
    }

    /**
     * @brief Handles all node formatting
     * @param node
     * @param indent indentation level, starting at 0, increment by 1
     * @param after_newline when false, do not add prefix
     * @return
     */
    std::string format(const Ark::internal::Node& node, std::size_t indent, bool after_newline);

    std::string formatComment(const std::string& comment, std::size_t indent) const;

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

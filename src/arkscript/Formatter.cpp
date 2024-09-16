#include <Ark/Constants.hpp>
#include <CLI/Formatter.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#include <Ark/Files.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Compiler/Common.hpp>

using namespace Ark;
using namespace Ark::internal;

Formatter::Formatter(bool dry_run) :
    m_dry_run(dry_run), m_parser(/* debug= */ 0, /* interpret= */ false), m_updated(false)
{}

Formatter::Formatter(std::string filename, const bool dry_run) :
    m_filename(std::move(filename)), m_dry_run(dry_run), m_parser(/* debug= */ 0, /* interpret= */ false), m_updated(false)
{}

void Formatter::run()
{
    try
    {
        const std::string code = Utils::readFile(m_filename);
        m_parser.process(m_filename, code);
        processAst(m_parser.ast());
        warnIfCommentsWereRemoved(code, ARK_NO_NAME_FILE);

        m_updated = code != m_output;
    }
    catch (const CodeError& e)
    {
        Diagnostics::generate(e);
    }
}

void Formatter::runWithString(const std::string& code)
{
    try
    {
        m_parser.process(ARK_NO_NAME_FILE, code);
        processAst(m_parser.ast());
        warnIfCommentsWereRemoved(code, ARK_NO_NAME_FILE);

        m_updated = code != m_output;
    }
    catch (const CodeError& e)
    {
        Diagnostics::generate(e);
    }
}

const std::string& Formatter::output() const
{
    return m_output;
}

bool Formatter::codeModified() const
{
    return m_updated;
}

void Formatter::processAst(const Node& ast)
{
    // remove useless surrounding begin (generated by the parser)
    if (isBeginBlock(ast))
    {
        std::size_t previous_line = 0;
        for (std::size_t i = 1, end = ast.constList().size(); i < end; ++i)
        {
            const Node node = ast.constList()[i];
            if (node.line() - previous_line > 1 && !m_output.empty())
                m_output += "\n";
            previous_line = lineOfLastNodeIn(node);
            m_output += format(node, 0, false) + "\n";
        }
    }
    else
        m_output = format(ast, 0, false);

    if (!m_dry_run)
    {
        std::ofstream stream(m_filename);
        stream << m_output;
    }
}

void Formatter::warnIfCommentsWereRemoved(const std::string& original_code, const std::string& filename)
{
    if (std::ranges::count(original_code, '#') != std::ranges::count(m_output, '#'))
    {
        fmt::println(
            "{}: one or more comments from the original source code seem to have been removed by mistake while formatting {}",
            fmt::styled("Warning", fmt::fg(fmt::color::dark_orange)),
            filename != ARK_NO_NAME_FILE ? filename : "file");
        fmt::println("Please fill an issue on GitHub: https://github.com/ArkScript-lang/Ark");
    }
}

bool Formatter::isListStartingWithKeyword(const Node& node, const Keyword keyword)
{
    return node.isListLike() && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Keyword && node.constList()[0].keyword() == keyword;
}

bool Formatter::isBeginBlock(const Node& node)
{
    return isListStartingWithKeyword(node, Keyword::Begin);
}

bool Formatter::isFuncDef(const Node& node)
{
    return isListStartingWithKeyword(node, Keyword::Fun);
}

bool Formatter::isFuncCall(const Node& node)
{
    return node.isListLike() && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Symbol;
}

bool Formatter::isPlainValue(const Node& node)
{
    switch (node.nodeType())
    {
        case NodeType::Symbol: [[fallthrough]];
        case NodeType::Number: [[fallthrough]];
        case NodeType::String: [[fallthrough]];
        case NodeType::Field: return true;

        default:
            return false;
    }
}

std::size_t Formatter::lineOfLastNodeIn(const Node& node)
{
    if (node.isListLike() && !node.constList().empty())
    {
        std::size_t child_line = lineOfLastNodeIn(node.constList().back());
        if (child_line < node.line())
            return node.line();
        return child_line;
    }
    return node.line();
}

bool Formatter::shouldSplitOnNewline(const Node& node)
{
    const std::string formatted = format(node, 0, false);
    const std::string::size_type sz = formatted.find_first_of('\n');

    const bool is_long_line = !((sz < FormatterConfig.LongLineLength || (sz == std::string::npos && formatted.size() < FormatterConfig.LongLineLength)));
    if (node.comment().empty() && (isBeginBlock(node) || isFuncCall(node)))
        return false;
    if (is_long_line || (node.isListLike() && node.constList().size() > 1) || !node.comment().empty())
        return true;
    return false;
}

std::string Formatter::format(const Node& node, std::size_t indent, bool after_newline)
{
    std::string output;
    if (!node.comment().empty())
    {
        output += formatComment(node.comment(), indent);
        after_newline = true;
    }
    if (after_newline)
        output += prefix(indent);

    switch (node.nodeType())
    {
        case NodeType::Symbol:
            output += node.string();
            break;
        case NodeType::Capture:
            output += "&" + node.string();
            break;
        case NodeType::Keyword:
            output += std::string(keywords[static_cast<std::size_t>(node.keyword())]);
            break;
        case NodeType::String:
            output += fmt::format("\"{}\"", node.string());
            break;
        case NodeType::Number:
            output += fmt::format("{}", node.number());
            break;
        case NodeType::List:
            output += formatBlock(node, indent, after_newline);
            break;
        case NodeType::Spread:
            output += fmt::format("...{}", node.string());
            break;
        case NodeType::Field:
        {
            std::string field = format(node.constList()[0], indent, false);
            for (std::size_t i = 1, end = node.constList().size(); i < end; ++i)
                field += "." + format(node.constList()[i], indent, false);
            output += field;
            break;
        }
        case NodeType::Macro:
            output += formatMacro(node, indent);
            break;
        case NodeType::Unused:
            break;
    }

    if (!node.commentAfter().empty())
        output += " " + formatComment(node.commentAfter(), /* indent= */ 0);

    return output;
}

std::string Formatter::formatComment(const std::string& comment, const std::size_t indent) const
{
    std::string output = prefix(indent);
    for (std::size_t i = 0, end = comment.size(); i < end; ++i)
    {
        output += comment[i];
        if (comment[i] == '\n' && i != end - 1)
            output += prefix(indent);
    }

    return output;
}

std::string Formatter::formatBlock(const Node& node, const std::size_t indent, const bool after_newline)
{
    if (node.constList().empty())
        return "()";

    const Node first = node.constList().front();
    if (first.nodeType() == NodeType::Keyword)
    {
        switch (first.keyword())
        {
            case Keyword::Fun:
                return formatFunction(node, indent);
            case Keyword::Let:
                [[fallthrough]];
            case Keyword::Mut:
                [[fallthrough]];
            case Keyword::Set:
                return formatVariable(node, indent);
            case Keyword::If:
                return formatCondition(node, indent);
            case Keyword::While:
                return formatLoop(node, indent);
            case Keyword::Begin:
                return formatBegin(node, indent, after_newline);
            case Keyword::Import:
                return formatImport(node, indent);
            case Keyword::Del:
                return formatDel(node, indent);
        }
        // HACK: should never reach, but the compiler insists that the function doesn't return in every code path
        return "";
    }
    return formatCall(node, indent);
}

std::string Formatter::formatFunction(const Node& node, const std::size_t indent)
{
    const Node args_node = node.constList()[1];
    const Node body_node = node.constList()[2];

    std::string formatted_args;

    if (!args_node.comment().empty())
    {
        formatted_args += "\n";
        formatted_args += formatComment(args_node.comment(), indent + 1);
        formatted_args += prefix(indent + 1);
    }
    else
        formatted_args += " ";

    if (args_node.isListLike())
    {
        bool comment_in_args = false;
        std::string args;
        for (std::size_t i = 0, end = args_node.constList().size(); i < end; ++i)
        {
            const Node arg_i = args_node.constList()[i];
            if (!arg_i.comment().empty())
                comment_in_args = true;

            args += format(arg_i, indent + (comment_in_args ? 1 : 0), comment_in_args);
            if (i != end - 1)
                args += comment_in_args ? '\n' : ' ';
        }

        formatted_args += fmt::format("({}{})", (comment_in_args ? "\n" : ""), args);
    }
    else
        formatted_args += format(args_node, indent, false);

    if (!shouldSplitOnNewline(body_node) && args_node.comment().empty())
        return fmt::format("(fun{} {})", formatted_args, format(body_node, indent + 1, false));
    return fmt::format("(fun{}\n{})", formatted_args, format(body_node, indent + 1, true));
}

std::string Formatter::formatVariable(const Node& node, const std::size_t indent)
{
    std::string keyword = std::string(keywords[static_cast<std::size_t>(node.constList()[0].keyword())]);

    const Node body_node = node.constList()[2];
    std::string formatted_body = format(body_node, indent + 1, false);

    if (!shouldSplitOnNewline(body_node) || isFuncDef(body_node))
        return fmt::format("({} {} {})", keyword, format(node.constList()[1], indent, false), formatted_body);
    return fmt::format("({} {}\n{})", keyword, format(node.constList()[1], indent, false), format(node.constList()[2], indent + 1, true));
}

std::string Formatter::formatCondition(const Node& node, const std::size_t indent, const bool is_macro)
{
    const Node cond_node = node.constList()[1];
    const Node then_node = node.constList()[2];

    bool cond_on_newline = false;
    std::string formatted_cond = format(cond_node, indent + 1, false);
    if (formatted_cond.find('\n') != std::string::npos)
        cond_on_newline = true;

    std::string if_cond_formatted = fmt::format(
        "({}if{}{}",
        is_macro ? "$" : "",
        cond_on_newline ? "\n" : " ",
        formatted_cond);

    const bool split_then_newline = shouldSplitOnNewline(then_node);

    // (if cond then)
    if (node.constList().size() == 3)
    {
        if (cond_on_newline || split_then_newline)
            return fmt::format("{}\n{})", if_cond_formatted, format(then_node, indent + 1, true));
        return fmt::format("{} {})", if_cond_formatted, format(then_node, indent + 1, false));
    }
    // (if cond then else)
    return fmt::format(
        "{}\n{}\n{}{})",
        if_cond_formatted,
        format(then_node, indent + 1, true),
        format(node.constList()[3], indent + 1, true),
        node.constList()[3].commentAfter().empty() ? "" : ("\n" + prefix(indent)));
}

std::string Formatter::formatLoop(const Node& node, const std::size_t indent)
{
    const Node cond_node = node.constList()[1];
    const Node body_node = node.constList()[2];

    bool cond_on_newline = false;
    std::string formatted_cond = format(cond_node, indent + 1, false);
    if (formatted_cond.find('\n') != std::string::npos)
        cond_on_newline = true;

    if (cond_on_newline || shouldSplitOnNewline(body_node))
        return fmt::format(
            "(while{}{}\n{})",
            cond_on_newline ? "\n" : " ",
            formatted_cond,
            format(body_node, indent + 1, true));
    return fmt::format(
        "(while {} {})",
        formatted_cond,
        format(body_node, indent + 1, false));
}

std::string Formatter::formatBegin(const Node& node, const std::size_t indent, const bool after_newline)
{
    // only the keyword begin is present
    if (node.constList().size() == 1)
        return "{}";

    std::string output = "{\n";
    std::size_t previous_line = 0;
    // skip begin keyword
    for (std::size_t i = 1, end = node.constList().size(); i < end; ++i)
    {
        const Node child = node.constList()[i];
        // we want to preserve the node grouping by the user, but remove useless duplicate new line
        // but that shouldn't apply to the first node of the block
        if (child.line() - previous_line > 1 && i > 1)
            output += "\n";
        previous_line = lineOfLastNodeIn(child);

        output += format(child, indent + (after_newline ? 1 : 0), true);
        if (i != end - 1)
            output += "\n";
    }
    output += " }";
    return output;
}

std::string Formatter::formatImport(const Node& node, const std::size_t indent)
{
    const Node package_node = node.constList()[1];
    std::string package;

    if (!package_node.comment().empty())
        package += "\n" + formatComment(package_node.comment(), indent + 1) + prefix(indent + 1);
    else
        package += " ";

    for (std::size_t i = 0, end = package_node.constList().size(); i < end; ++i)
    {
        package += format(package_node.constList()[i], indent + 1, false);
        if (i != end - 1)
            package += ".";
    }

    const Node symbols = node.constList()[2];
    if (symbols.nodeType() == NodeType::Symbol && symbols.string() == "*")
        package += ":*";
    else  // symbols is a list
    {
        for (const auto& sym : symbols.constList())
        {
            if (sym.comment().empty())
                package += " :" + sym.string();
            else
                package += "\n" + formatComment(sym.comment(), indent + 1) + prefix(indent + 1) + ":" + sym.string();
            if (!sym.commentAfter().empty())
                package += " " + formatComment(sym.commentAfter(), /* indent= */ 0);
        }
    }

    return fmt::format("(import{})", package);
}

std::string Formatter::formatDel(const Node& node, const std::size_t indent)
{
    std::string formatted_sym = format(node.constList()[1], indent + 1, false);
    if (formatted_sym.find('\n') != std::string::npos)
        return fmt::format("(del\n{})", formatted_sym);
    return fmt::format("(del {})", formatted_sym);
}

std::string Formatter::formatCall(const Node& node, const std::size_t indent)
{
    bool is_list = false;
    if (!node.constList().empty() && node.constList().front().nodeType() == NodeType::Symbol &&
        node.constList().front().string() == "list")
        is_list = true;

    bool is_multiline = false;

    std::vector<std::string> formatted_args;
    for (std::size_t i = 1, end = node.constList().size(); i < end; ++i)
    {
        formatted_args.push_back(format(node.constList()[i], indent, false));
        // if we have at least one argument taking multiple lines, split them all on their own line
        if (formatted_args.back().find('\n') != std::string::npos || !node.constList()[i].commentAfter().empty())
            is_multiline = true;
    }

    std::string output = is_list ? "[" : ("(" + format(node.constList()[0], indent, false));
    for (std::size_t i = 0, end = formatted_args.size(); i < end; ++i)
    {
        const std::string formatted_node = formatted_args[i];
        if (is_multiline)
            output += "\n" + format(node.constList()[i + 1], indent + 1, true);
        else
            output += (is_list && i == 0 ? "" : " ") + formatted_node;
    }
    if (!node.constList().back().commentAfter().empty())
        output += "\n" + prefix(indent);
    output += is_list ? "]" : ")";
    return output;
}

std::string Formatter::formatMacro(const Node& node, const std::size_t indent)
{
    if (isListStartingWithKeyword(node, Keyword::If))
        return formatCondition(node, indent, /* is_macro= */ true);

    std::string output;
    // because some macro call like ($undef ...) are considered macros and we shouldn't confuse them and write ($ $undef ...)
    if (!node.constList().empty() && node.constList().front().nodeType() == NodeType::Symbol && node.constList().front().string().starts_with('$'))
        output = "(";
    else
        output = "($ ";

    bool after_newline = false;
    for (std::size_t i = 0, end = node.constList().size(); i < end; ++i)
    {
        output += format(node.constList()[i], indent + 1, after_newline);
        after_newline = false;

        if (!node.constList()[i].commentAfter().empty())
        {
            output += "\n";
            after_newline = true;
        }
        else if (i != end - 1)
            output += " ";
    }

    return output + ")";
}

// check if a given character is a valid hex char
#define CHECK_IF_HEXCHAR(chr) (('a' <= chr && chr <= 'f') || ('A' <= chr && chr <= 'F') || ('0' <= chr && chr <= '9'))

inline TokenType Lexer::guessType(const std::string& value) noexcept
{
    if (value.empty())
        return TokenType::Mismatch;

    // assuming we already detected ()[]{}, strings, shorthands and comments
    if (Utils::isDouble(value))  // works on (\+|-)?[[:digit:]]+(\.[[:digit:]]+)?([e|E](\+|-)?[[:digit]]+)?
        return TokenType::Number;
    else if (isOperator(value))
        return TokenType::Operator;
    else if (isKeyword(value))
        return TokenType::Keyword;
    else if (value[0] == '&' && value.size() > 1 && isIdentifier(value))
        return TokenType::Capture;
    else if (value[0] == '.' && value.size() > 1 && isIdentifier(value))
        return TokenType::GetField;
    else if (isIdentifier(value))
        return TokenType::Identifier;
    else if (value.size() > 3 && value[0] == value[1] && value[1] == value[2] && value[2] == '.')
        return TokenType::Spread;
    return TokenType::Mismatch;
}

inline bool Lexer::isKeyword(const std::string& value) noexcept
{
    return std::find(keywords.begin(), keywords.end(), value) != keywords.end();
}

inline bool Lexer::isIdentifier(const std::string& value) noexcept
{
    return utf8valid(value.c_str());
}

inline bool Lexer::isOperator(const std::string& value) noexcept
{
    return std::find(operators.begin(), operators.end(), value) != operators.end();
}

inline bool Lexer::endOfControlChar(const std::string& sequence, char next) noexcept
{
    switch (sequence[0])
    {
        case 'x':
            // \x[any number of hex digits]
            // if it's not a hex digit then it's most likely the end for us
            return !CHECK_IF_HEXCHAR(next);

        case 'u':
            return sequence.size() == 5;

        case 'U':
            return sequence.size() == 6;

        case '"':
        case 'n':
        case 'a':
        case 'b':
        case 't':
        case 'r':
        case 'f':
        case '\\':
        case '0':
            return true;
    }
    return false;
}

inline void Lexer::throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col, const std::string& context)
{
    std::vector<std::string> ctx = Utils::splitString(context, '\n');

    std::stringstream ss;
    ss << message << "\n";

    for (int i=3; i > -3; i--)
    {
        int iline = static_cast<int>(line);

        if (iline - i >= 0 && iline - i < ctx.size())
            // + 1 to display real lines numbers
            ss << std::setw(5) << (iline - i + 1) << " | " << ctx[iline - i] << "\n";
        if (i == 0)  // line of the error
        {
            ss << "      | ";
            // padding of spaces
            for (std::size_t j=0; (match.size() > col) ? false : (j + 1 < col - match.size()); ++j)
                ss << " ";
            // show the error
            for (std::size_t j=0; (match.size() > col) ? (j < ctx[line].size()) : (j < match.size()); ++j)
                ss << "^";
            ss << "\n";
        }
    }

    throw Ark::SyntaxError(ss.str());
}

#undef CHECK_FIRST_CHAR
#undef CHECK_IF_HEXCHAR

inline bool Lexer::isKeyword(const std::string& value)
{
    return std::find(keywords.begin(), keywords.end(), value) != keywords.end();
}

inline void Lexer::throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col)
{
    throw std::runtime_error("TokenizingError: " + message + "\nAt " +
        Ark::Utils::toString(line) + ":" + Ark::Utils::toString(col) +
        " (" + match + ")"
    );
}
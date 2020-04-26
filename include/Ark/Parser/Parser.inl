inline void Parser::except(bool pred, const std::string& message, internal::Token token)
{
    if (!pred)
        throwParseError(message, token);
}

inline void Parser::throwParseError(const std::string& message, internal::Token token)
{
    throw std::runtime_error("ParseError: " + message + "\nAt " +
        Ark::Utils::toString(token.line) + ":" + Ark::Utils::toString(token.col) +
        " `" + token.token + "' (" + internal::tokentype_string[static_cast<unsigned>(token.type)] + ")" +
        ((m_file != "FILE") ? " in file " + m_file : "")
    );
}

inline void Parser::throwParseError_(const std::string& message)
{
    throw std::runtime_error("ParseError: " + message);
}
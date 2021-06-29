inline internal::NodeType similar_nodetype_from_tokentype(internal::TokenType tt)
{
    if (tt == internal::TokenType::Capture)
        return internal::NodeType::Capture;
    else if (tt == internal::TokenType::GetField)
        return internal::NodeType::GetField;
    else if (tt == internal::TokenType::Spread)
        return internal::NodeType::Spread;

    return internal::NodeType::Symbol;
}

inline void Parser::expect(bool pred, const std::string& message, internal::Token token)
{
    if (!pred)
        throwParseError(message, token);
}

inline void Parser::throwParseError(const std::string& message, internal::Token token)
{
    std::stringstream ss;
    ss << message << "\nGot TokenType::" << internal::tokentype_string[static_cast<unsigned>(token.type)] << "\n";

    if (m_file != ARK_NO_NAME_FILE)
        ss << "In file " << m_file << "\n";
    ss << internal::makeTokenBasedErrorCtx(token.token, token.line, token.col, m_code);

    throw Ark::ParseError(ss.str());
}

inline void Parser::throwParseError_(const std::string& message)
{
    throw Ark::ParseError(message);
}

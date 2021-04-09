inline NodeType similar_nodetype_from_tokentype(internal::TokenType tt)
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
    std::vector<std::string> ctx = Utils::splitString(m_code, '\n');

    std::stringstream ss;
    ss << message << "\n";
    if (m_file != ARK_NO_NAME_FILE)
        ss << "In file " << m_file << "\n";
    ss << "On line " << (token.line + 1) << ":" << token.col << ", got TokenType::";
    ss << internal::tokentype_string[static_cast<unsigned>(token.type)] << "\n";

    for (int i=3; i > -3; --i)
    {
        int iline = static_cast<int>(token.line);
        if (iline - i >= 0 && iline - i < static_cast<int>(ctx.size()))  // cast to int, we'll never have a size > 2^32-1
            // + 1 to display real lines numbers
            ss << std::setw(5) << (iline - i + 1) << " | " << ctx[iline - i] << "\n";
        if (i == 0)  // line of the error
        {
            ss << "      | ";
            // padding of spaces
            for (std::size_t j=0; (token.token.size() > token.col) ? false : (j < token.col); ++j)
                ss << " ";
            // show the error
            for (std::size_t j=0; (token.token.size() > token.col) ? (j < ctx[token.line].size()) : (j < token.token.size()); ++j)
                ss << "^";
            ss << "\n";
        }
    }

    throw Ark::ParseError(ss.str());
}

inline void Parser::throwParseError_(const std::string& message)
{
    throw Ark::ParseError(message);
}

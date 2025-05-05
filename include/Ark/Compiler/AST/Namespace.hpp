#ifndef ARK_COMPILER_AST_NAMESPACE_HPP
#define ARK_COMPILER_AST_NAMESPACE_HPP

namespace Ark::internal
{
    class Node;

    struct Namespace
    {
        std::string name;
        bool is_glob;                      // (import package:*)
        bool with_prefix;                  // (import package)
        std::vector<std::string> symbols;  // (import package :a :b)
        std::shared_ptr<Node> ast;
    };

    inline bool operator==(const Namespace& A, const Namespace& B)
    {
        return A.name == B.name && A.is_glob == B.is_glob &&
            A.with_prefix == B.with_prefix;
    }

    inline bool operator<([[maybe_unused]] const Namespace&, [[maybe_unused]] const Namespace&)
    {
        return true;
    }
}

#endif  // ARK_COMPILER_AST_NAMESPACE_HPP

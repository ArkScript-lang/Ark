#ifndef ARK_COMPILER_AST_NAMESPACE_HPP
#define ARK_COMPILER_AST_NAMESPACE_HPP

#include <string>
#include <vector>
#include <memory>

#include <Ark/Compiler/AST/Import.hpp>

namespace Ark::internal
{
    class Node;

    struct Namespace
    {
        std::string name;     ///< Prefix for the namespace, eg egg
        std::string package;  ///< Complete package name, eg foo.bar.egg
        std::vector<std::string> symbols;
        std::vector<Import> imports;
        std::shared_ptr<Node> ast;
    };

    inline bool operator==(const Namespace& A, const Namespace& B)
    {
        return A.name == B.name && A.ast.get() == B.ast.get();
    }

    inline bool operator<([[maybe_unused]] const Namespace&, [[maybe_unused]] const Namespace&)
    {
        return true;
    }
}

#endif  // ARK_COMPILER_AST_NAMESPACE_HPP

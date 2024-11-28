#ifndef ARK_PACKAGE_HPP
#define ARK_PACKAGE_HPP

#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Import.hpp>

namespace Ark::internal
{
    // TODO store something better than just the AST (AST+what we are importing as private/public/namespaced... vs all)
    //      so that we can remember the order in which we encountered imports.
    struct Package
    {
        Node ast;
        Import import;
        bool has_been_processed = false;  ///< Set to false for source code files, to indicate they need parsing. True for modules (.arkm), as we cannot parse those
    };
}

#endif  // ARK_PACKAGE_HPP

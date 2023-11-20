#ifndef ARK_MODULE_HPP
#define ARK_MODULE_HPP

#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    // TODO store something better than just the AST (AST+what we are importing as private/public/namespaced... vs all)
    //      so that we can remember the order in which we encountered imports.
    struct Module
    {
        Node ast;
        bool has_been_processed = false;  // TODO document this
    };
}

#endif  // ARK_MODULE_HPP

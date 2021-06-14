#include <Ark/Compiler/Optimizer.hpp>

namespace Ark
{
    using namespace internal;

    // flags for our visitor
    using flag_t = Optimizer::flag_t;
    constexpr flag_t flag_is_condition = 1 << 0;
    constexpr flag_t flag_is_func_arg  = 1 << 1;
    constexpr flag_t flag_is_terminal  = 1 << 2;
    constexpr flag_t flag_is_function  = 1 << 3;
    constexpr flag_t flag_is_stored    = 1 << 4;
    constexpr flag_t flag_is_if_block  = 1 << 5;

    Optimizer::Optimizer(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {}

    void Optimizer::feed(const Node& ast)
    {
        m_ast = ast;

        if (m_debug > 1)
            std::clog << "AST before optimizations\n" << m_ast << "\n";

        visit(m_ast, 0);

        if (m_debug > 1)
            std::clog << "AST after optimizations\n" << m_ast << "\n";

        // if (m_options & FeatureRemoveUnusedVars)
        //     remove_unused();
    }

    const Node& Optimizer::ast() const noexcept
    {
        return m_ast;
    }

    void Optimizer::visit(Node& node, std::size_t depth, flag_t flags)
    {
        /// DEBUG
        if (m_debug > 1)
        {
            std::clog << depth << " - ";

            std::clog << ((flags & flag_is_condition) ? 'C' : ' ');
            std::clog << ((flags & flag_is_func_arg)  ? 'A' : ' ');
            std::clog << ((flags & flag_is_terminal)  ? 'T' : ' ');
            std::clog << ((flags & flag_is_function)  ? 'F' : ' ');
            std::clog << ((flags & flag_is_stored)    ? 'S' : ' ');
            std::clog << ((flags & flag_is_if_block)  ? 'I' : ' ');

            std::clog << " : " << "Node(" << typeToString(node) << ") => " << node << "\n";
        }
        /// END DEBUG

        switch (node.nodeType())
        {
            case NodeType::List:
                for (std::size_t i = 0, end = node.list().size(); i < end; ++i)
                {
                    // flags for a subnode is based on the parent node
                    flag_t new_flags = determineFlags(node, i)
                                        | (flags & flag_is_stored)
                                        | (flags & flag_is_func_arg);
                    visit(node.list()[i], depth + 1, new_flags);
                }
                break;

            default:
                if (depth > 1 && flags == 0)
                {
                    Node tmp(Keyword::Pop);
                    tmp.push_back(node);
                    node = tmp;
                }
                m_flags.pop_back();
                break;
        }
    }

    flag_t Optimizer::determineFlags(const Node& node, std::size_t child_pos)
    {
        flag_t flags = 0;

        return flags;
    }

    #pragma region "to clean up later"

    void Optimizer::remove_unused()
    {
        // do not handle non-list nodes
        if (m_ast.nodeType() != NodeType::List)
            return;

        runOnGlobalScopeVars(m_ast, [this](Node& node, Node& parent, int idx){
            m_sym_appearances[node.constList()[1].string()] = 0;
        });
        countOccurences(m_ast);

        // logic: remove piece of code with only 1 reference, if they aren't function calls
        runOnGlobalScopeVars(m_ast, [this](Node& node, Node& parent, int idx){
            std::string name = node.constList()[1].string();
            // a variable was only declared and never used
            if (m_sym_appearances.find(name) != m_sym_appearances.end() && m_sym_appearances[name] == 1
                && parent.list()[idx].list()[2].nodeType() != NodeType::List)
                parent.list().erase(parent.list().begin() + idx);  // erase the node from the list
        });
    }

    void Optimizer::runOnGlobalScopeVars(Node& node, const std::function<void(Node&, Node&, int)>& func)
    {
        int i = static_cast<int>(node.constList().size());
        // iterate only on the first level, using reverse iterators to avoid copy-delete-move to nowhere
        for (auto it = node.list().rbegin(); it != node.list().rend(); ++it)
        {
            i--;

            if (it->constList().size() > 0 && it->constList()[0].nodeType() == NodeType::Keyword)
            {
                Keyword kw = it->constList()[0].keyword();

                // eliminate nested begin blocks
                if (kw == Keyword::Begin)
                {
                    runOnGlobalScopeVars(*it, func);
                    // skip let/ mut detection
                    continue;
                }
                // check if it's a let/mut declaration
                else if (kw == Keyword::Let || kw == Keyword::Mut)
                    func(*it, node, i);
            }
        }
    }

    void Optimizer::countOccurences(Node& node)
    {
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::Capture)
        {
            std::string name = node.string();
            // check if it's the name of something declared in global scope
            if (m_sym_appearances.find(name) != m_sym_appearances.end())
                m_sym_appearances[name]++;
        }
        else if (node.nodeType() == NodeType::List)
        {
            // iterate over children
            for (std::size_t i = 0, end = node.constList().size(); i != end; ++i)
                countOccurences(node.list()[i]);
        }
    }

    #pragma endregion
}
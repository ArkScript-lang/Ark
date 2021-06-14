#include <Ark/Compiler/Optimizer.hpp>

namespace Ark
{
    using namespace internal;

    // flags for our visitor
    using flag_t = Optimizer::flag_t;
    constexpr flag_t flag_terminal_node = 1 << 0;
    constexpr flag_t flag_stored_node   = 1 << 1;
    constexpr flag_t flag_avoid_pop     = 1 << 2;
    constexpr flag_t flag_function_call = 1 << 3;

    Optimizer::Optimizer(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {}

    void Optimizer::feed(const Node& ast)
    {
        m_ast = ast;

        visit(m_ast, 0);

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
            std::clog << depth << " - Node(" << typeToString(node) << ") => " << node << "\n";
        }
        /// END DEBUG

        switch (node.nodeType())
        {
            case NodeType::List:
                for (std::size_t i = 0, end = node.list().size(); i < end; ++i)
                {
                    // flags for a subnode is based on the parent node
                    flag_t flags = determineFlags(node, i);
                    visit(node.list()[i], depth + 1, flags);
                }
                break;

            default:
                /// TODO
                break;
        }
    }

    flag_t Optimizer::determineFlags(const Node& node, std::size_t child_pos)
    {
        flag_t flags = 0;

        if (node.nodeType() == NodeType::List && !node.constList().empty())
        {
            switch (node.constList()[0].nodeType())
            {
                case NodeType::Symbol:  // can be a function call
                    flags |= flag_function_call;
                    break;

                case NodeType::Capture:  // in a function argument bloc
                    flags |= flag_avoid_pop;
                    break;

                case NodeType::GetField:  // this is impossible
                    break;

                case NodeType::Keyword:
                {
                    Keyword k = node.constList()[0].keyword();
                    switch (k)
                    {
                        case Keyword::Fun:
                            break;

                        case Keyword::Let:
                        case Keyword::Mut:
                        case Keyword::Set:
                            flags |= flag_stored_node;
                            break;

                        case Keyword::If:
                            // should store only the condition
                            break;

                        case Keyword::While:
                            // same
                            break;

                        case Keyword::Begin:
                        case Keyword::Import:
                        case Keyword::Quote:
                        case Keyword::Del:  // TODO find something to do with those
                            break;
                    }
                    break;
                }

                case NodeType::String:
                case NodeType::Number:  // problems, can not call a value
                    break;

                case NodeType::Macro:  // skip, impossible
                case NodeType::Spread:
                case NodeType::Unused:
                    break;
            }
        }

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
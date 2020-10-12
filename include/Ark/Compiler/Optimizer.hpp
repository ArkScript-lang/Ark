#ifndef ark_compiler_optimizer
#define ark_compiler_optimizer

#include <functional>
#include <unordered_map>
#include <string>
#include <cinttypes>

#include <Ark/Compiler/Node.hpp>
#include <Ark/Constants.hpp>

namespace Ark
{
    /**
     * @brief The ArkScript AST optimizer
     * 
     */
    class Optimizer
    {
    public:
        /**
         * @brief Construct a new Optimizer
         * 
         */
        Optimizer(uint16_t options) noexcept;

        void feed(const internal::Node& ast);

        const internal::Node& ast() const noexcept;

    private:
        internal::Node m_ast;
        uint16_t m_options;
        std::unordered_map<std::string, unsigned> m_symAppearances;

        // iterate over the AST and remove unused top level functions and constants
        void remove_unused();
        void run_on_global_scope_vars(internal::Node& node, const std::function<void(internal::Node&, internal::Node&, int)>& func);
        void count_occurences(const internal::Node& node);
    };
}

#endif  // ark_compiler_optimizer
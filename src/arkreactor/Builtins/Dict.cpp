#include <Ark/Builtins/Builtins.hpp>

#include <fmt/format.h>

#include <Ark/VM/Value/Dict.hpp>
#include <Ark/TypeChecker.hpp>

namespace Ark::internal::Builtins::Dict
{
    /**
     * @name dict
     * @brief Creates a dictionary from a set of arguments, grouping them 2 by 2 to create (key, value) pairs
     * @param args... the arguments of the function
     * =begin
     * (let d (dict "key" "value" 5 12))
     * (print d)  # {key: value, 5: 12}
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value dict(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() % 2 != 0)
            throw std::runtime_error(
                fmt::format("dict: require an even number of argument to construct (key, value) pairs and create a dictionary. Got {} argument{}",
                            n.size(),
                            n.size() > 1 ? "s" : ""));

        internal::Dict dict;
        for (std::size_t i = 0, end = n.size(); i < end; i += 2)
            dict.set(n[i], n[i + 1]);

        return Value(std::move(dict));
    }

    Value get(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Dict, ValueType::Any))
            throw types::TypeCheckingError(
                "dict:get",
                { { types::Contract { { types::Typedef("dictionary", ValueType::Dict),
                                        types::Typedef("key", ValueType::Any) } } } },
                n);

        return n[0].dictRef().get(n[1]);
    }

    Value add(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Dict, ValueType::Any, ValueType::Any))
            throw types::TypeCheckingError(
                "dict:add",
                { { types::Contract { { types::Typedef("dictionary", ValueType::Dict),
                                        types::Typedef("key", ValueType::Any),
                                        types::Typedef("value", ValueType::Any) } } } },
                n);

        n[0].dictRef().set(n[1], n[2]);
        return nil;
    }

    Value contains(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Dict, ValueType::Any))
            throw types::TypeCheckingError(
                "dict:contains?",
                { { types::Contract { { types::Typedef("dictionary", ValueType::Dict),
                                        types::Typedef("key", ValueType::Any) } } } },
                n);

        return n[0].dictRef().contains(n[1]) ? trueSym : falseSym;
    }

    Value remove(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Dict, ValueType::Any))
            throw types::TypeCheckingError(
                "dict:remove",
                { { types::Contract { { types::Typedef("dictionary", ValueType::Dict),
                                        types::Typedef("key", ValueType::Any) } } } },
                n);

        n[0].dictRef().remove(n[1]);
        return nil;
    }

    Value keys(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Dict))
            throw types::TypeCheckingError(
                "dict:keys",
                { { types::Contract { { types::Typedef("dictionary", ValueType::Dict) } } } },
                n);

        return Value(n[0].dictRef().keys());
    }

    Value size(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Dict))
            throw types::TypeCheckingError(
                "dict:size",
                { { types::Contract { { types::Typedef("dictionary", ValueType::Dict) } } } },
                n);

        return Value(static_cast<int>(n[0].dictRef().size()));
    }
}

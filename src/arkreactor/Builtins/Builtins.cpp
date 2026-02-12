#include <limits>
#include <numbers>
#include <cmath>

#include <Ark/Constants.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins
{
    extern const Value falseSym = Value(ValueType::False);
    extern const Value trueSym = Value(ValueType::True);
    extern const Value nil = Value(ValueType::Nil);
    extern const Value platform = Value(ARK_PLATFORM_NAME);

    namespace Mathematics
    {
        extern const Value pi_ = Value(std::numbers::pi);
        extern const Value e_ = Value(std::exp(1.0));
        extern const Value tau_ = Value(std::numbers::pi * 2.0);
        extern const Value inf_ = Value(std::numeric_limits<double>::infinity());
        extern const Value nan_ = Value(std::numeric_limits<double>::signaling_NaN());
    }

    /**
     * @name slice
     * @brief Slice a list or string given a start, an end, and an optional step size
     * @param container list or string
     * @param start number, included
     * @param end number, excluded
     * @param step number, default 1
     * =begin
     * (let d (dict "key" "value" 5 12))
     * (print d)  # {key: value, 5: 12}
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value slice(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() < 3 || n.size() > 4 ||
            (n[1].valueType() != ValueType::Number && n[1].valueType() != ValueType::Nil) ||
            (n[2].valueType() != ValueType::Number && n[2].valueType() != ValueType::Nil) ||
            (n.size() == 4 && n[3].valueType() != ValueType::Number))
            throw types::TypeCheckingError(
                "slice",
                { { types::Contract {
                        { types::Typedef("container", { ValueType::List, ValueType::String }),
                          types::Typedef("start", ValueType::Number),
                          types::Typedef("end", ValueType::Number) } },
                    types::Contract {
                        { types::Typedef("container", { ValueType::List, ValueType::String }),
                          types::Typedef("start", ValueType::Number),
                          types::Typedef("end", ValueType::Number),
                          types::Typedef("step", ValueType::Number) } } } },
                n);

        const bool is_list = n[0].valueType() == ValueType::List;
        const std::size_t container_size = is_list ? n[0].constList().size() : n[0].string().size();

        const long start = n[1].valueType() == ValueType::Number ? static_cast<long>(n[1].number()) : 0;
        const std::size_t i = static_cast<std::size_t>(start < 0 ? static_cast<long>(container_size) + start : start);

        if (i >= container_size)
            VM::throwVMError(
                ErrorKind::Index,
                fmt::format("{} out of range {} (length {})", start, n[0].toString(*vm, /* show_as_code= */ true), container_size));

        const long end = n[2].valueType() == ValueType::Number ? static_cast<long>(n[2].number()) : static_cast<long>(container_size);
        const std::size_t j = std::min(container_size, static_cast<std::size_t>(end < 0 ? static_cast<long>(container_size) + end : end));

        const long step = n.size() == 4 ? static_cast<long>(n[3].number()) : 1L;
        if (step == 0)
            throw Error("slice: a step of 0 is illegal");

        std::size_t a = step > 0 ? i : j - 1;
        const std::size_t b = step > 0 ? j : i;
        const std::size_t increments = static_cast<std::size_t>(std::abs(step));

        if (is_list)
        {
            Value output(ValueType::List);
            while ((step > 0 && a < b) || (step < 0 && a >= b))
            {
                output.push_back(n[0].constList()[a]);

                if (step > 0)
                    a += increments;
                else if (a >= increments)
                    a -= increments;
                else
                    break;  // step < 0 and 'increments' is bigger than 'a'
            }
            return output;
        }

        std::string output;
        while ((step > 0 && a < b) || (step < 0 && a >= b))
        {
            output += n[0].string()[a];

            if (step > 0)
                a += increments;
            else if (a >= increments)
                a -= increments;
            else
                break;  // step < 0 and 'increments' is bigger than 'a'
        }
        return Value(output);
    }

    extern const std::vector<std::pair<std::string, Value>> builtins = {
        // builtin variables or constants
        { "false", falseSym },
        { "true", trueSym },
        { "nil", nil },

        // List
        { "builtin__list:reverse", Value(List::reverseList) },
        { "builtin__list:find", Value(List::findInList) },
        { "builtin__list:slice", Value(List::sliceList) },
        { "builtin__list:sort", Value(List::sort_) },
        { "builtin__list:fill", Value(List::fill) },
        { "builtin__list:setAt", Value(List::setListAt) },

        // IO
        { "print", Value(IO::print) },
        { "puts", Value(IO::puts_) },
        { "input", Value(IO::input) },
        { "builtin__io:writeFile", Value(IO::writeFile) },
        { "builtin__io:appendToFile", Value(IO::appendToFile) },
        { "builtin__io:readFile", Value(IO::readFile) },
        { "builtin__io:readLinesFile", Value(IO::readLinesFile) },
        { "builtin__io:fileExists?", Value(IO::fileExists) },
        { "builtin__io:listFiles", Value(IO::listFiles) },
        { "builtin__io:dir?", Value(IO::isDirectory) },
        { "builtin__io:makeDir", Value(IO::makeDir) },
        { "builtin__io:removeFile", Value(IO::removeFile) },

        // Time
        { "time", Value(Time::timeSinceEpoch) },

        // System
        { "builtin__sys:platform", platform },
        { "builtin__sys:exec", Value(System::system_) },
        { "builtin__sys:sleep", Value(System::sleep) },
        { "builtin__sys:exit", Value(System::exit_) },
        { "assert", Value(System::assert_) },

        // String
        { "format", Value(String::format) },
        { "builtin__string:find", Value(String::findSubStr) },
        { "builtin__string:removeAt", Value(String::removeAtStr) },
        { "builtin__string:ord", Value(String::ord) },
        { "builtin__string:chr", Value(String::chr) },
        { "builtin__string:setAt", Value(String::setStringAt) },

        // Mathematics
        { "builtin__math:exp", Value(Mathematics::exponential) },
        { "builtin__math:ln", Value(Mathematics::logarithm) },
        { "builtin__math:ceil", Value(Mathematics::ceil_) },
        { "builtin__math:floor", Value(Mathematics::floor_) },
        { "builtin__math:round", Value(Mathematics::round_) },
        { "builtin__math:NaN?", Value(Mathematics::isnan_) },
        { "builtin__math:Inf?", Value(Mathematics::isinf_) },
        { "builtin__math:pi", Mathematics::pi_ },
        { "builtin__math:e", Mathematics::e_ },
        { "builtin__math:tau", Mathematics::tau_ },
        { "builtin__math:Inf", Mathematics::inf_ },
        { "builtin__math:NaN", Mathematics::nan_ },
        { "builtin__math:cos", Value(Mathematics::cos_) },
        { "builtin__math:sin", Value(Mathematics::sin_) },
        { "builtin__math:tan", Value(Mathematics::tan_) },
        { "builtin__math:arccos", Value(Mathematics::acos_) },
        { "builtin__math:arcsin", Value(Mathematics::asin_) },
        { "builtin__math:arctan", Value(Mathematics::atan_) },
        { "builtin__math:cosh", Value(Mathematics::cosh_) },
        { "builtin__math:sinh", Value(Mathematics::sinh_) },
        { "builtin__math:tanh", Value(Mathematics::tanh_) },
        { "builtin__math:acosh", Value(Mathematics::acosh_) },
        { "builtin__math:asinh", Value(Mathematics::asinh_) },
        { "builtin__math:atanh", Value(Mathematics::atanh_) },
        { "random", Value(Mathematics::random) },

        // Async
        { "async", Value(Async::async) },
        { "await", Value(Async::await) },

        // Dict
        { "dict", Value(Dict::dict) },
        { "builtin__dict:get", Value(Dict::get) },
        { "builtin__dict:add", Value(Dict::add) },
        { "builtin__dict:contains", Value(Dict::contains) },
        { "builtin__dict:remove", Value(Dict::remove) },
        { "builtin__dict:keys", Value(Dict::keys) },
        { "builtin__dict:size", Value(Dict::size) },

        // Bytecode
        { "disassemble", Value(Bytecode::disassemble) },

        { "slice", Value(slice) },

        // Operators that can also be used as builtins
        { "+", Value(Operators::add) },
        { "-", Value(Operators::sub) },
        { "*", Value(Operators::mul) },
        { "/", Value(Operators::div) },
        { "mod", Value(Operators::mod) },
        { "toNumber", Value(Operators::toNumber) },
        { "toString", Value(Operators::toString) },
        { "<", Value(Operators::lessThan) },
        { "<=", Value(Operators::lessOrEq) },
        { ">", Value(Operators::greaterThan) },
        { ">=", Value(Operators::greaterOrEq) },
        { "=", Value(Operators::eq) },
        { "!=", Value(Operators::notEq) },
        { "not", Value(Operators::not_) },
        { "len", Value(Operators::len) },
        { "empty?", Value(Operators::isEmpty) },
        { "nil?", Value(Operators::isNil) },
        { "tail", Value(Operators::tail) },
        { "head", Value(Operators::head) },
        { "@", Value(Operators::at) },
        { "@@", Value(Operators::atAt) },
        { "type", Value(Operators::type) }
    };
}

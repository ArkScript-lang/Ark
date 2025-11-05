#include <limits>
#include <numbers>
#include <cmath>

#include <Ark/Constants.hpp>
#include <Ark/Builtins/Builtins.hpp>

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
        { "disassemble", Value(Bytecode::disassemble) }
    };
}

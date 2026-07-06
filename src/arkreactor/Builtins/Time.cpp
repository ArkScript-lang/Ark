#include <Ark/Builtins/Builtins.hpp>

#include <Ark/VM/DefaultValues.hpp>
#include <Ark/VM/Value/Dict.hpp>
#include <Ark/TypeChecker.hpp>

#undef abs
#include <chrono>
#include <iomanip>

#include <newlib/gmtime_r.h>

namespace Ark::internal::Builtins::Time
{
    /**
     * @name time
     * @brief Return the time of the computer since epoch, in seconds, with at least milliseconds precision
     * =begin
     * (time)  # 1627134107.837558031082153
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value timeSinceEpoch(std::vector<Value>& n [[maybe_unused]], VM* vm [[maybe_unused]])
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        return Value(static_cast<double>(microseconds.count()) / 1000000);
    }

    Value timestampToDate(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "asUTCDate",
                { { types::Contract {
                    { types::Typedef("timestamp", ValueType::Number) } } } },
                n);

        nl_tm calendar_time {};
        nl_gmtime_r(static_cast<long long>(n[0].number()), &calendar_time);

        int week = calendar_time.tm_wday;
        if (week == 0)  // Sunday
            week = 6;
        else
            --week;  // 0-5: Monday-Saturday

        internal::Dict dict;
        const double ms = n[0].number() - static_cast<double>(static_cast<long long>(n[0].number()));

        dict.set(Value("millisecond"), Value(static_cast<int>(1000.0 * ms)));
        dict.set(Value("second"), Value(calendar_time.tm_sec));
        dict.set(Value("minute"), Value(calendar_time.tm_min));
        dict.set(Value("hour"), Value(calendar_time.tm_hour));
        dict.set(Value("day"), Value(calendar_time.tm_mday));
        dict.set(Value("month"), Value(calendar_time.tm_mon + 1));
        dict.set(Value("year"), Value(calendar_time.tm_year + 1900));
        dict.set(Value("week_day"), Value(week));
        dict.set(Value("year_day"), Value(calendar_time.tm_yday));
        dict.set(Value("is_dst"), calendar_time.tm_isdst ? True : False);

        return Value(std::move(dict));
    }

    constexpr int64_t floor_div(const int64_t a, const int64_t b)
    {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    }

    int64_t makeTimestamp(const int tm_sec, const int tm_min, const int tm_hour, const int tm_mday, const int tm_mon, const int tm_year)
    {
        constexpr int MonthsPerYear = 12;
        static const std::array<int, MonthsPerYear> cumulative_days = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

        const long year = tm_year + tm_mon / MonthsPerYear;
        int64_t result = (year - 1970) * 365 + cumulative_days[static_cast<std::size_t>(tm_mon % MonthsPerYear)];
        result += floor_div(year - 1968, 4) - floor_div(year - 1900, 100) + floor_div(year - 1600, 400);
        if ((year % 4) == 0 &&
            ((year % 100) != 0 || (year % 400) == 0) &&
            (tm_mon % MonthsPerYear) < 2)
            result--;
        result += tm_mday - 1;
        result *= 24;
        result += tm_hour;
        result *= 60;
        result += tm_min;
        result *= 60;
        result += tm_sec;
        return result;
    }

    Value parseDate(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String) && !types::check(n, ValueType::String, ValueType::String))
            throw types::TypeCheckingError(
                "parseDate",
                { { types::Contract {
                        { types::Typedef("date", ValueType::String) } },
                    types::Contract {
                        { types::Typedef("date", ValueType::String),
                          types::Typedef("format", ValueType::String) } } } },
                n);

        std::tm t = {};
        std::istringstream ss(n[0].string());
        ss >> std::get_time(&t, n.size() == 1 ? "%Y-%m-%dT%H:%M:%S" : n[1].string().c_str());

        if (ss.fail())
            return Nil;
        return Value(makeTimestamp(t.tm_sec, t.tm_min, t.tm_hour, t.tm_mday, t.tm_mon, t.tm_year + 1900));
    }
}

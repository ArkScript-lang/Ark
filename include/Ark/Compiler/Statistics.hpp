#ifndef ARK_COMPILER_STATISTICS_HPP
#define ARK_COMPILER_STATISTICS_HPP

#include <string>
#include <chrono>
#include <vector>

#include <Ark/Utils/Platform.hpp>

namespace Ark::internal
{
    enum class Stats : unsigned
    {
#define X(name) name,
#include "Statistics.x"

#undef X
        Count
    };

    constexpr std::array StatNames = {
#define X(name) #name,
#include "Statistics.x"

#undef X
    };

    class ARK_API Statistics
    {
    public:
        Statistics() = default;

        /**
         * @brief Register an event with the number of times it happened
         *
         * @param name
         * @param quantity
         */
        void count(Stats name, long quantity);

        [[nodiscard]] long getCount(Stats name) const;

        /**
         * @brief Register an event with the time it took
         *
         * @param name
         * @param quantity
         */
        void time(const std::string& name, std::chrono::nanoseconds quantity);

        [[nodiscard]] std::string asJson() const noexcept;

    private:
        struct Measure
        {
            std::string name;
            std::chrono::nanoseconds duration;
        };

        std::vector<Measure> m_measures;
        std::array<long, static_cast<std::size_t>(Stats::Count)> m_counts { 0 };
    };
}

#endif  // ARK_COMPILER_STATISTICS_HPP

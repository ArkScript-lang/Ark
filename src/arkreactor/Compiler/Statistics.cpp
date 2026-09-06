#include <Ark/Compiler/Statistics.hpp>

#include <fmt/format.h>

namespace Ark::internal
{
    void Statistics::count(const Stats name, const long quantity)
    {
        m_counts[static_cast<std::size_t>(name)] = quantity;
    }

    long Statistics::getCount(const Stats name) const
    {
        return m_counts[static_cast<std::size_t>(name)];
    }

    void Statistics::time(const std::string& name, const std::chrono::nanoseconds quantity)
    {
        m_measures.emplace_back(name, quantity);
    }

    std::string Statistics::asJson() const noexcept
    {
        std::string timings;
        for (const auto& [k, v] : m_measures)
        {
            const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(v);
            timings += fmt::format("{:?}: {},", k, micros.count());
        }

        std::string counts;
        for (std::size_t i = 0; i < m_counts.size(); ++i)
        {
            const long x = m_counts[i];
            if (i > 0)
                counts += ",";
            counts += fmt::format("{:?}: {}", StatNames[i], x);
        }

        return fmt::format(R"({{ "timings": {{ {} "unit": "us" }}, "counts": {{ {} }} }})", timings, counts);
    }
}

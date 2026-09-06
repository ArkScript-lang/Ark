#include <Ark/Compiler/Pass.hpp>
#include <utility>

namespace Ark::internal
{
    Pass::Pass(std::string name, const unsigned debug_level, Statistics* stats_collector) :
        m_logger(std::move(name), debug_level), m_stats(stats_collector)
    {}

    void Pass::configureLogger(std::ostream& os)
    {
        m_logger.configureOutputStream(&os);
    }

    void Pass::statIncrementCount(const Stats name, const long delta) const
    {
        if (m_stats)
            m_stats->count(name, m_stats->getCount(name) + delta);
    }

    void Pass::addStat(const Stats name, const long quantity) const
    {
        if (m_stats)
            m_stats->count(name, quantity);
    }

    void Pass::addStat(const std::string& name, const std::chrono::nanoseconds quantity) const
    {
        if (m_stats)
            m_stats->time(name, quantity);
    }
}

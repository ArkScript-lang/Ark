#include <Ark/Log.hpp>

namespace Ark
{
    Logger::Logger(const std::string& file) :
        m_file(file),
        m_level(LogLevel::Normal)
    {}

    Logger::~Logger()
    {
        if (m_file != "" && !m_buffer.empty())  // write to file
        {
            std::ofstream file(m_file);
            for (auto& line: m_buffer)
                file << line << std::endl;
            file.close();
        }
    }

    void Logger::setLevel(LogLevel level)
    {
        m_level = level;
    }

    void Logger::colorize(LogColor color)
    {
        if (m_file == "")
        {
            switch (color)
            {
            case LogColor::Red:
                std::cout << termcolor::red;
                break;
            
            case LogColor::Blue:
                std::cout << termcolor::cyan;
                break;

            case LogColor::Green:
                std::cout << termcolor::green;
                break;
            
            case LogColor::Yellow:
                std::cout << termcolor::yellow;
                break;
            
            case LogColor::Magenta:
                std::cout << termcolor::magenta;
                break;

            case LogColor::Nope:
            default:
                std::cout << termcolor::reset;
                break;
            }
        }
    }

    Logger logger = Logger();
}

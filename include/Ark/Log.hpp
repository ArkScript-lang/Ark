#ifndef ark_log
#define ark_log

#include <string>
#include <iostream>
#include <termcolor.hpp>
#include <fmt/format.hpp>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

namespace Ark
{
    enum class LogLevel
    {
        Dont,
        Normal
    };

    enum class LogColor
    {
        Nope,
        Red,
        Blue,
        Green,
        Yellow,
        Magenta
    };

    class Logger
    {
    public:
        Logger(const std::string& file="");
        ~Logger();

        Logger(const Logger&) = delete;

        void setLevel(LogLevel level);

        template<typename... Args> void log(const Args&... args) { write2(LogColor::Nope, "[  Log  ] " + loggerData(), args...); }
        template<typename... Args> void log(const std::string& fmt, const Args&... args) { write(LogColor::Nope, "[  Log  ] " + loggerData() + fmt, args...); }

        template<typename... Args> void warn(const Args&... args) { write2(LogColor::Yellow, "[Warning] " + loggerData(), args...); }
        template<typename... Args> void warn(const std::string& fmt, const Args&... args) { write(LogColor::Yellow, "[Warning] " + loggerData() + fmt, args...); }

        template<typename... Args> void info(const Args&... args) { write2(LogColor::Blue, "[  Info ] " + loggerData(), args...); }
        template<typename... Args> void info(const std::string& fmt, const Args&... args) { write(LogColor::Blue, "[  Info ] " + loggerData() + fmt, args...); }

        template<typename... Args> void error(const Args&... args) { write2(LogColor::Red, "[ Error ] " + loggerData(), args...); }
        template<typename... Args> void error(const std::string& fmt, const Args&... args) { write(LogColor::Red, "[ Error ] " + loggerData() + fmt, args...); }

        template<typename... Args> void success(const Args&... args) { write2(LogColor::Green, "[Success] " + loggerData(), args...); }
        template<typename... Args> void success(const std::string& fmt, const Args&... args) { write(LogColor::Green, "[Success] " + loggerData() + fmt, args...); }

        template<typename... Args> void data(const Args&... args) { write2(LogColor::Magenta, "[  Data ] " + loggerData(), args...); }
        template<typename... Args> void data(const std::string& fmt, const Args&... args) { write(LogColor::Magenta, "[  Data ] " + loggerData() + fmt, args...); }

    private:
        std::string m_file;
        LogLevel m_level;
        std::vector<std::string> m_buffer;

        inline std::string loggerData()
        {
            /*std::time_t t = std::time(0);   // get time now
            std::tm* now = std::localtime(&t);
            std::stringstream stream;
            stream << "(" << std::put_time(now, "%c %Z") << ") ";
            return stream.str();*/

            return "";
        }

        void colorize(LogColor color);

        template<typename... Args>
        void write(LogColor color, const std::string& fmt, const Args&... args)
        {
            if (m_level == LogLevel::Dont)
                return;
            
            colorize(color);

            std::string f = rj::format(fmt, args...);

            // write to terminal
            if (m_file == "")
                std::cout << f << termcolor::reset << std::endl;
            else  // write to buffer to write to file
                m_buffer.push_back(f);
        }

        template <typename T>
        inline std::string make_str(const T& arg)
        {
            std::ostringstream os;
            os << arg;
            return os.str();
        }

        template <typename T, typename... Args>
        std::string make_str(const T& arg, const Args&... args)
        {
            return make_str(arg) + " " + make_str(args...);
        }

        template<typename... Args>
        void write2(LogColor color, const Args&... args)
        {
            if (m_level == LogLevel::Dont)
                return;
            
            colorize(color);

            std::string f = make_str(args...);

            // write to terminal
            if (m_file == "")
                std::cout << f << termcolor::reset << std::endl;
            else  // write to buffer to write to file
                m_buffer.push_back(f);
        }
    };

    extern Logger logger;
}

#endif  // ark_log

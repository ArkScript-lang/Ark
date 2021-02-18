/**
 * @file Log.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript logger
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef INCLUDE_ARK_LOG_HPP
#define INCLUDE_ARK_LOG_HPP

#include <string>
#include <iostream>
#include <termcolor.hpp>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <utility>

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

    /**
     * @brief A basic logger
     * 
     */
    class Logger
    {
    public:
        explicit Logger(const std::string& file="");
        ~Logger();

        Logger(const Logger&) = delete;

        void setLevel(LogLevel level);

        template<typename... Args> void log(Args&&... args)     { write(LogColor::Nope,    "[  Log  ]", std::forward<Args>(args)...); }
        template<typename... Args> void warn(Args&&... args)    { write(LogColor::Yellow,  "[Warning]", std::forward<Args>(args)...); }
        template<typename... Args> void info(Args&&... args)    { write(LogColor::Blue,    "[  Info ]", std::forward<Args>(args)...); }
        template<typename... Args> void error(Args&&... args)   { write(LogColor::Red,     "[ Error ]", std::forward<Args>(args)...); }
        template<typename... Args> void success(Args&&... args) { write(LogColor::Green,   "[Success]", std::forward<Args>(args)...); }
        template<typename... Args> void data(Args&&... args)    { write(LogColor::Magenta, "[  Data ]", std::forward<Args>(args)...); }

    private:
        std::string m_file;
        LogLevel m_level;
        std::vector<std::string> m_buffer;

        void colorize(LogColor color);

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
        void write(LogColor color, const Args&... args)
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

#endif

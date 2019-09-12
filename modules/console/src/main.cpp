#include <string>
#include <random>
#include <chrono>

#include <Ark/Module.hpp>

namespace ArkConsole
{
    Value clear(const std::vector<Value>& n)
    {
    #if defined(OS_WINDOWS)
        COORD topLeft  = { 0, 0 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO screen;
        DWORD written;

        GetConsoleScreenBufferInfo(console, &screen);
        FillConsoleOutputCharacterA(
            console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
        FillConsoleOutputAttribute(
            console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
            screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
        SetConsoleCursorPosition(console, topLeft);
    #elif defined(OS_LINUX)
        std::cout << "\x1B[2J\x1B[H";
    #endif
        return Nil;
    }
}

MAKE_ENTRY_POINT()

ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;
    map["clear"] = ArkConsole::clear;

    return map;
}
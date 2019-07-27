#ifndef ark_ark
#define ark_ark

#include <Ark/Exceptions.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Utils.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Compiler/Compiler.hpp>

namespace Ark
{
    void compile(bool debug, const std::string& file, const std::string& output);
    void vm(bool debug, const std::string& file);
    void bcr(const std::string& file);
    void run(const std::string& file, bool debug, bool recompile=false);
}

#endif  // ark_ark
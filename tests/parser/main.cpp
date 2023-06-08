#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Exceptions.hpp>

#include <iostream>
#include <iomanip>
#include <string>

using namespace Ark;
using namespace Ark::internal;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected at least one argument: filename" << std::endl;
        return 1;
    }

    std::string filename(argv[1]);

    try
    {
        Parser parser;
        parser.processFile(filename);

        // print imports too, so that we can be sure that we are parsing and reading them correctly
        const auto& ast = parser.ast().constList();
        for (auto it = ast.begin() + 1, end = ast.end(); it != end; ++it)
            std::cout << *it << std::endl;

        const auto& imports = parser.imports();

        if (!imports.empty())
            std::cout << "\n";
        for (std::size_t i = 0, end = imports.size(); i < end; ++i)
        {
            Import data = imports[i];
            std::cout << i << ") " << data.package;
            if (data.isBasic())
                std::cout << " (basic)";
            else if (data.isGlob())
                std::cout << " (glob)";
            else
            {
                std::cout << " ( ";
                for (const std::string& sym : data.symbols)
                    std::cout << sym << " ";
                std::cout << ")";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }
    catch (const CodeError& e)
    {
        Diagnostics::generate(e);
    }

    return 0;
}

#include <benchmark/benchmark.h>

#include <string>
#include <fstream>

#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Compiler/Welder.hpp>
#include <Ark/State.hpp>
#include <Ark/VM/VM.hpp>

#define ARK_CREATE_RUNTIME_BENCH(name)                                       \
    void name(benchmark::State& s)                                           \
    {                                                                        \
        Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") }); \
        state.doFile(get_resource("runtime/" #name ".ark"));                 \
        for (auto _ : s)                                                     \
        {                                                                    \
            Ark::VM vm(state);                                               \
            benchmark::DoNotOptimize(vm.run());                              \
        }                                                                    \
    }                                                                        \
    BENCHMARK(name)->Unit(benchmark::kMillisecond)

std::string get_resource(const std::string& path)
{
    return (ARK_TESTS_ROOT "tests/benchmarks/resources/") + path;
}

ARK_CREATE_RUNTIME_BENCH(quicksort);
ARK_CREATE_RUNTIME_BENCH(ackermann)->Iterations(50);
ARK_CREATE_RUNTIME_BENCH(fibonacci)->Iterations(100);
// ARK_CREATE_RUNTIME_BENCH(man_or_boy);
ARK_CREATE_RUNTIME_BENCH(builtins);
ARK_CREATE_RUNTIME_BENCH(binary_trees);
ARK_CREATE_RUNTIME_BENCH(for_sum);
ARK_CREATE_RUNTIME_BENCH(create_closure)->Iterations(500);
ARK_CREATE_RUNTIME_BENCH(create_list)->Iterations(500);
ARK_CREATE_RUNTIME_BENCH(create_list_with_ref)->Iterations(500);
ARK_CREATE_RUNTIME_BENCH(n_queens)->Iterations(50);

// --------------------------------------------
// parser benchmarks
// --------------------------------------------

std::string readFile(const std::string& filename)
{
    std::ifstream stream(filename);
    std::string code((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return code;
}

constexpr int simple = 0, medium = 1, big = 2, bigger = 3;

std::string select_file(const long selection)
{
    switch (selection)
    {
        case simple:
            return "simple.ark";
        case medium:
            return "medium.ark";
        case big:
            return "big.ark";
        case bigger:
            return "bigger.ark";
        default:
            return "no name provided error";
    }
}

// cppcheck-suppress constParameterCallback
static void BM_Parse(benchmark::State& state)
{
    using namespace std::string_literals;

    const long selection = state.range(0);
    const std::string filename = get_resource("parser/"s + select_file(selection));
    const std::string code = readFile(filename);
    long linesCount = 0;
    for (const char c : code)
        if (c == '\n')
            ++linesCount;  // cppcheck-suppress useStlAlgorithm

    long long nodes = 0;
    long long lines = 0;

    for (auto _ : state)
    {
        Ark::internal::Parser parser(0);
        parser.process(ARK_NO_NAME_FILE, code);

        nodes += parser.ast().constList().size();
        lines += linesCount;
    }

    state.counters["nodesRate"] = benchmark::Counter(nodes, benchmark::Counter::kIsRate);
    state.counters["nodesAvg"] = benchmark::Counter(nodes, benchmark::Counter::kAvgThreads);
    state.counters["uselessLines/sec"] = benchmark::Counter(lines, benchmark::Counter::kIsRate);
}

BENCHMARK(BM_Parse)->Name("New parser - Simple - 39 nodes")->Arg(simple)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Parse)->Name("New parser - Medium - 83 nodes")->Arg(medium)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Parse)->Name("New parser - Big - 665 nodes")->Arg(big)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Parse)->Name("New parser - Bigger")->Arg(bigger)->Unit(benchmark::kMillisecond);

// cppcheck-suppress constParameterCallback
static void BM_Welder(benchmark::State& state)
{
    using namespace std::string_literals;

    const long selection = state.range(0);
    const std::string filename = get_resource("parser/"s + select_file(selection));

    for (auto _ : state)
    {
        Ark::Welder welder(0, { ARK_TESTS_ROOT "lib" });
        benchmark::DoNotOptimize(welder.computeASTFromFile(filename));
        benchmark::DoNotOptimize(welder.generateBytecode());
    }
}

BENCHMARK(BM_Welder)->Name("Welder - Simple - 39 nodes")->Arg(simple)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Welder)->Name("Welder - Medium - 83 nodes")->Arg(medium)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Welder)->Name("Welder - Big - 665 nodes")->Arg(big)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Welder)->Name("Welder - Bigger")->Arg(bigger)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

#include <benchmark/benchmark.h>

#include <string>
#include <fstream>

#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Compiler/Welder.hpp>
#include <Ark/VM/State.hpp>
#include <Ark/VM/VM.hpp>

// cppcheck-suppress constParameterCallback
void quicksort(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/resources/runtime/quicksort.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(quicksort)->Unit(benchmark::kMillisecond);

// cppcheck-suppress constParameterCallback
void ackermann(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/resources/runtime/ackermann.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(ackermann)->Unit(benchmark::kMillisecond)->Iterations(50);

// cppcheck-suppress constParameterCallback
void fibonacci(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/resources/runtime/fibonacci.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(fibonacci)->Unit(benchmark::kMillisecond)->Iterations(100);

// cppcheck-suppress constParameterCallback
void man_or_boy(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/resources/runtime/man_or_boy_test.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(man_or_boy)->Unit(benchmark::kMillisecond);

// cppcheck-suppress constParameterCallback
void builtins(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/resources/runtime/builtins.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(builtins)->Unit(benchmark::kMillisecond);

// --------------------------------------------
// parser benchmarks
// --------------------------------------------

std::string readFile(const std::string& filename)
{
    std::ifstream stream(filename);
    std::string code((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return code;
}

constexpr int simple = 0, medium = 1, big = 2;

// cppcheck-suppress constParameterCallback
static void BM_Parse(benchmark::State& state)
{
    using namespace std::string_literals;

    const long selection = state.range(0);
    const std::string filename = "tests/benchmarks/resources/parser/"s + (selection == simple ? "simple.ark" : (selection == medium ? "medium.ark" : "big.ark"));
    const std::string code = readFile(filename);
    long linesCount = 0;
    for (const char c : code)
        if (c == '\n')
            ++linesCount;

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

// cppcheck-suppress constParameterCallback
static void BM_Welder(benchmark::State& state)
{
    using namespace std::string_literals;

    const long selection = state.range(0);
    const std::string filename = "tests/benchmarks/resources/parser/"s + (selection == simple ? "simple.ark" : (selection == medium ? "medium.ark" : "big.ark"));

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

BENCHMARK_MAIN();

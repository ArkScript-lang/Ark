#include <benchmark/benchmark.h>

#include <string>

#include <Ark/VM/State.hpp>
#include <Ark/VM/VM.hpp>

// cppcheck-suppress constParameterCallback
void quicksort(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/quicksort.ark");

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
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/ackermann.ark");

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
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/fibonacci.ark");

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
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/man_or_boy_test.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(man_or_boy)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

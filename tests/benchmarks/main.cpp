#include <benchmark/benchmark.h>

#include <string>

#include <Ark/VM/State.hpp>
#include <Ark/VM/VM.hpp>

// cppcheck-suppress constParameterCallback
void ark_quicksort(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/quicksort.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(ark_quicksort)->Unit(benchmark::kMillisecond);

// cppcheck-suppress constParameterCallback
void ark_ackermann(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/ackermann.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(ark_ackermann)->Unit(benchmark::kMillisecond)->Iterations(50);

// cppcheck-suppress constParameterCallback
void ark_fibonacci(benchmark::State& s)
{
    Ark::State state;
    state.doFile(std::string(ARK_TESTS_ROOT) + "tests/benchmarks/fibonacci.ark");

    for (auto _ : s)
    {
        Ark::VM vm(state);
        benchmark::DoNotOptimize(vm.run());
    }
}
BENCHMARK(ark_fibonacci)->Unit(benchmark::kMillisecond)->Iterations(100);

BENCHMARK_MAIN();

#include <benchmark/benchmark.h>
#include <Ark/Ark.hpp>

static void Ackermann_3_6(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        Ark::VM vm;
        vm.feed("tests/ackermann.arkc");
        vm.run();
    }
}

static void let_a_42(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        Ark::VM vm;
        vm.feed("tests/test-let.arkc");
        vm.run();
    }
}

BENCHMARK(Ackermann_3_6)->Unit(benchmark::kMillisecond);
BENCHMARK(let_a_42);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}
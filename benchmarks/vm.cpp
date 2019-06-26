#include <benchmark/benchmark.h>
#include <Ark/Ark.hpp>

static void BM_VM(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        Ark::VM vm;
        vm.feed("tests/ackermann.arkc");
        vm.run();
    }
}

BENCHMARK(BM_VM);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}
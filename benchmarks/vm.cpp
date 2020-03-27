#include <benchmark/benchmark.h>
#include <Ark/Ark.hpp>

static void Ackermann_3_6_ark(benchmark::State& state)
{
    Ark::State ark_state;
    ark_state.doFile("examples/ackermann.ark");
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
        vm.run();
    }
}

static void Fibo_28_ark(benchmark::State& state)
{
    Ark::State ark_state;
    ark_state.doFile("examples/fibo.ark");
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
        vm.run();
    }
}

static void List_Alloc(benchmark::State& state)
{
    Ark::State ark_state;
    ark_state.doFile("examples/list_alloc.ark");
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
        vm.run();
    }
}

static void vm_boot(benchmark::State& state)
{
    Ark::State ark_state;
    ark_state.doFile("examples/__arkscript_cache__/fibo.arkc");
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
    }
}

BENCHMARK(Ackermann_3_6_ark)->Unit(benchmark::kMillisecond);
BENCHMARK(Fibo_28_ark)->Unit(benchmark::kMillisecond);
BENCHMARK(List_Alloc)->Unit(benchmark::kMillisecond);
BENCHMARK(vm_boot)->Unit(benchmark::kNanosecond);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

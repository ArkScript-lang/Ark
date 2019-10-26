#include <benchmark/benchmark.h>
#include <Ark/Ark.hpp>

unsigned ack(unsigned m, unsigned n)
{
    if (m > 0)
    {
        if (n == 0)
            return ack(m - 1, 1);
        else
            return ack(m - 1, ack(m, n - 1));
    }
    else
        return n + 1;
}

// --------------------------------------------------

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

static void Ackermann_3_6_cpp(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(ack(3, 6));
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
BENCHMARK(Ackermann_3_6_cpp)->Unit(benchmark::kMillisecond);
BENCHMARK(vm_boot)->Unit(benchmark::kNanosecond);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

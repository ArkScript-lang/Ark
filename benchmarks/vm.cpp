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
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
        vm.doFile("examples/ackermann.ark");
    }
}

static void Fibo_28_ark(benchmark::State& state)
{
    Ark::State ark_state;
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
        vm.doFile("examples/fibo.ark");
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
    while (state.KeepRunning())
    {
        Ark::VM vm(&ark_state);
        vm.feed("examples/__arkscript_cache__/fibo.arkc");
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
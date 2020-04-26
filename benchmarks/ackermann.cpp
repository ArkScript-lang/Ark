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

static void Ackermann_3_6_cpp(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(ack(3, 6));
    }
}

BENCHMARK(Ackermann_3_6_cpp)->Unit(benchmark::kMillisecond);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

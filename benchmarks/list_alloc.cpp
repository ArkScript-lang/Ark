#include <benchmark/benchmark.h>
#include <Ark/Ark.hpp>
#include <vector>

void allocate()
{
    std::vector<int> data;
    for (int i=0; i < 1000; ++i)
        data.push_back(0);
}

// --------------------------------------------------

static void List_Alloc(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        allocate();
    }
}

BENCHMARK(List_Alloc)->Unit(benchmark::kMicrosecond);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

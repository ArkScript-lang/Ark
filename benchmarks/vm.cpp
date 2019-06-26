#include <benchmark/benchmark.h>
#include <Ark/Ark.hpp>

auto vm = [](benchmark::State& state)
{
    Ark::VM vm;
};

int main(int argc, char** argv)
{
    auto unit = benchmark::kMicrosecond;

    benchmark::RegisterBenchmark("vm", vm)->Unit(unit)->RangeMultiplier(100)->Range(10, 100000);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}
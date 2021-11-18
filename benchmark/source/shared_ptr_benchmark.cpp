#include "shared_ptr/shared_ptr.hpp"

#include <benchmark/benchmark.h>

static void BM_SomeFunction(benchmark::State& state)
{
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    wind::name();
  }
}
// Register the function as a benchmark
BENCHMARK(BM_SomeFunction);

BENCHMARK_MAIN();
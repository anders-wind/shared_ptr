#include <memory>

#include <benchmark/benchmark.h>
#include <shared_ptr/bias_shared_ptr.hpp>
#include <shared_ptr/local_shared_ptr.hpp>

template<typename FuncT>
void copyAndRelease(int64_t num_iteration, int64_t num_copies, const FuncT& generator)
{
    for (auto i = 0; i < num_iteration; i++) {
        auto ptr = generator(i);
        for (auto j = 0; j < num_copies; j++) {
            auto copy = ptr;
            benchmark::DoNotOptimize(copy);
        }
    }
}

static void BM_copy_and_release_local(benchmark::State& state)
{
    // Perform setup here
    for (auto _ : state) {
        // This code gets timed
        copyAndRelease(state.range(0),
                       state.range(1),
                       [](auto i) { return wind::regular::make_shared<int>(i * 2); });
    }
}

static void BM_copy_and_release_biased(benchmark::State& state)
{
    // Perform setup here
    for (auto _ : state) {
        // This code gets timed
        copyAndRelease(state.range(0),
                       state.range(1),
                       [](auto i) { return wind::bias::make_shared<int>(i * 2); });
    }
}

static void BM_copy_and_release_std(benchmark::State& state)
{
    // Perform setup here
    for (auto _ : state) {
        // This code gets timed
        copyAndRelease(
            state.range(0), state.range(1), [](auto i) { return std::make_shared<int>(i * 2); });
    }
}

// Register the function as a benchmark
BENCHMARK(BM_copy_and_release_local)->Ranges({{1 << 9, 8 << 10}, {128, 512}})->RangeMultiplier(2);
BENCHMARK(BM_copy_and_release_biased)->Ranges({{1 << 9, 8 << 10}, {128, 512}})->RangeMultiplier(2);
BENCHMARK(BM_copy_and_release_std)->Ranges({{1 << 9, 8 << 10}, {128, 512}})->RangeMultiplier(2);

BENCHMARK_MAIN();
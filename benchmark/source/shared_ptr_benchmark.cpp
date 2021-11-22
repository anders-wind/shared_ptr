#include <array>
#include <memory>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>
#include <shared_ptr/bias_shared_ptr.hpp>
#include <shared_ptr/local_shared_ptr.hpp>

// benchmark functions

template<typename FuncT>
void copying(int64_t num_copies, const FuncT& generator)
{
    auto ptr = generator();
    for (auto j = 0; j < num_copies; j++) {
        auto copy = ptr;
        benchmark::DoNotOptimize(copy);
    }
}

template<typename FuncT>
void copy_and_release(int64_t num_iteration, int64_t num_copies, const FuncT& generator)
{
    for (auto i = 0; i < num_iteration; i++) {
        auto ptr = generator(i);
        for (auto j = 0; j < num_copies; j++) {
            auto copy = ptr;
            benchmark::DoNotOptimize(copy);
        }
    }
}

template<typename FuncT>
void copy_and_release_many(int64_t num_iteration, int64_t num_copies, const FuncT& generator)
{
    using shared_ptr_type = typename std::invoke_result_t<FuncT, int>;

    for (auto i = 0; i < num_iteration; i++) {
        auto ptrs = std::array<shared_ptr_type, 5> {
            generator(i), generator(i), generator(i), generator(i), generator(i)};

        for (auto j = 0; j < num_copies; j++) {
            auto copy = ptrs[j % 5];
            benchmark::DoNotOptimize(copy);
        }
    }
}

template<typename FuncT>
void copy_back_and_forth_between_threads(int64_t num_iteration,
                                         int64_t num_copies,
                                         int64_t num_threads,
                                         const FuncT& generator)
{
    using shared_ptr_type = typename std::invoke_result_t<FuncT, int>;

    auto ptrs = std::vector<shared_ptr_type>();
    for (auto i = 0; i < num_copies; i++) {
        ptrs.push_back(generator(i));
    }

    // std::barrier sync_point(num_threads);
    auto threads = std::vector<std::thread>();
    for (auto i = 0; i < num_threads; i++) {
        threads.push_back(std::thread(
            [&ptrs, &num_iteration, &num_copies]()
            {
                auto local_ptrs = std::vector<shared_ptr_type>(num_copies);
                // sync_point.arrive_and_wait();

                for (auto i = 0; i < num_iteration; i++) {
                    for (auto j = 0; j < num_copies; j++) {
                        local_ptrs.at((j + i) % num_copies) = ptrs.at(j % num_copies);
                        // benchmark::DoNotOptimize(local_ptrs.at((j + i) % num_copies));
                    }
                }

                // sync_point.arrive_and_wait();
                benchmark::DoNotOptimize(local_ptrs);
            }));
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

// Specific benchmarks

// ===== copying =====

static void BM_copying_local(benchmark::State& state)
{
    for (auto _ : state) {
        copying(state.range(0), []() { return wind::regular::make_shared<int>(42); });
    }
}

static void BM_copying_biased(benchmark::State& state)
{
    for (auto _ : state) {
        copying(state.range(0), []() { return wind::bias::make_shared<int>(42); });
    }
}

static void BM_copying_std(benchmark::State& state)
{
    for (auto _ : state) {
        copying(state.range(0), []() { return std::make_shared<int>(42); });
    }
}

// ===== copy_and_release =====

static void BM_copy_and_release_local(benchmark::State& state)
{
    for (auto _ : state) {
        copy_and_release(
            state.range(0), 128, [](auto i) { return wind::regular::make_shared<int>(i * 2); });
    }
}

static void BM_copy_and_release_biased(benchmark::State& state)
{
    for (auto _ : state) {
        copy_and_release(
            state.range(0), 128, [](auto i) { return wind::bias::make_shared<int>(i * 2); });
    }
}

static void BM_copy_and_release_std(benchmark::State& state)
{
    for (auto _ : state) {
        copy_and_release(state.range(0), 128, [](auto i) { return std::make_shared<int>(i * 2); });
    }
}

// =====  copy_and_release_many =====

static void BM_copy_and_release_many_local(benchmark::State& state)
{
    for (auto _ : state) {
        copy_and_release_many(
            state.range(0), 128, [](auto i) { return wind::regular::make_shared<int>(i * 2); });
    }
}

static void BM_copy_and_release_many_biased(benchmark::State& state)
{
    for (auto _ : state) {
        copy_and_release_many(
            state.range(0), 128, [](auto i) { return wind::bias::make_shared<int>(i * 2); });
    }
}

static void BM_copy_and_release_many_std(benchmark::State& state)
{
    for (auto _ : state) {
        copy_and_release_many(
            state.range(0), 128, [](auto i) { return std::make_shared<int>(i * 2); });
    }
}

// ===== copy_back_and_forth_between_threads =====

static void BM_copy_back_and_forth_between_threads_biased(benchmark::State& state)
{
    for (auto _ : state) {
        copy_back_and_forth_between_threads(
            state.range(0), 128, 8, [](auto i) { return wind::bias::make_shared<int>(i * 2); });
    }
}

static void BM_copy_back_and_forth_between_threads_std(benchmark::State& state)
{
    for (auto _ : state) {
        copy_back_and_forth_between_threads(
            state.range(0), 128, 8, [](auto i) { return std::make_shared<int>(i * 2); });
    }
}

// Register benchmarks

BENCHMARK(BM_copying_local)->Range(1 << 4, 1 << 8);
BENCHMARK(BM_copying_biased)->Range(1 << 4, 1 << 8);
BENCHMARK(BM_copying_std)->Range(1 << 4, 1 << 8);

BENCHMARK(BM_copy_and_release_local)->Range(1 << 9, 8 << 10);
BENCHMARK(BM_copy_and_release_biased)->Range(1 << 9, 8 << 10);
BENCHMARK(BM_copy_and_release_std)->Range(1 << 9, 8 << 10);

BENCHMARK(BM_copy_and_release_many_local)->Range(1 << 9, 8 << 10);
BENCHMARK(BM_copy_and_release_many_biased)->Range(1 << 9, 8 << 10);
BENCHMARK(BM_copy_and_release_many_std)->Range(1 << 9, 8 << 10);

// local ofcourse does not work
BENCHMARK(BM_copy_back_and_forth_between_threads_biased)->Range(1 << 9, 8 << 10);
BENCHMARK(BM_copy_back_and_forth_between_threads_std)->Range(1 << 9, 8 << 10);

BENCHMARK_MAIN();
#include <array>
#include <iostream>
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
    constexpr int64_t number_of_ptrs = 32;

    for (int64_t i = 0; i < num_iteration; i++) {
        auto ptrs = std::array<shared_ptr_type, number_of_ptrs> {};
        for (auto ptr_i = 0; ptr_i < number_of_ptrs; ptr_i++) {
            ptrs.at(ptr_i) = generator(i);
        }

        for (int64_t j = 0; j < num_copies; j++) {
            auto copy = ptrs.at(static_cast<uint64_t>(j) % static_cast<uint64_t>(ptrs.size()));
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
    for (auto t = 0; t < num_threads; t++) {
        threads.push_back(std::thread(
            [&ptrs, &num_iteration, &num_copies]()
            {
                auto local_ptrs = std::vector<shared_ptr_type>(static_cast<size_t>(num_copies));
                auto local_ptrs2 = std::vector<shared_ptr_type>(static_cast<size_t>(num_copies));
                // sync_point.arrive_and_wait();

                for (uint64_t i = 0; i < static_cast<uint64_t>(num_iteration); i++) {
                    local_ptrs.at(i % static_cast<uint64_t>(num_copies)) =
                        ptrs.at(i % static_cast<uint64_t>(num_copies));
                    for (uint64_t j = 0; j < static_cast<uint64_t>(num_copies); j++) {
                        local_ptrs2.at((i * j) % static_cast<uint64_t>(num_copies)) =
                            local_ptrs.at((i + j) % static_cast<uint64_t>(num_copies));
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

static void bm_copying_local(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copying(state.range(0), []() { return wind::local::make_shared<int64_t>(42); });
    }
}

static void bm_copying_bias(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copying(state.range(0), []() { return wind::bias::make_shared<int64_t>(42); });
    }
}

static void bm_copying_std(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copying(state.range(0), []() { return std::make_shared<int64_t>(42); });
    }
}

// ===== copy_and_release =====

static void bm_copy_and_release_local(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_and_release(state.range(0), 128, [](auto i) { return wind::local::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_and_release_bias(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_and_release(state.range(0), 128, [](auto i) { return wind::bias::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_and_release_std(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_and_release(state.range(0), 128, [](auto i) { return std::make_shared<int64_t>(i * 2); });
    }
}

// =====  copy_and_release_many =====

static void bm_copy_and_release_many_local(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_and_release_many(state.range(0), 128, [](auto i) { return wind::local::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_and_release_many_bias(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_and_release_many(state.range(0), 128, [](auto i) { return wind::bias::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_and_release_many_std(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_and_release_many(state.range(0), 128, [](auto i) { return std::make_shared<int64_t>(i * 2); });
    }
}

// ===== copy_back_and_forth_between_threads =====

static void bm_copy_back_and_forth_between_threads_many_threads_many_copies_bias(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_back_and_forth_between_threads(
            state.range(0), 128, 128, [](auto i) { return wind::bias::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_back_and_forth_between_threads_many_threads_many_copies_std(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_back_and_forth_between_threads(
            state.range(0), 128, 128, [](auto i) { return std::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_back_and_forth_between_threads_many_threads_few_copies_bias(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_back_and_forth_between_threads(
            state.range(0), 2, 128, [](auto i) { return wind::bias::make_shared<int64_t>(i * 2); });
    }
}

static void bm_copy_back_and_forth_between_threads_many_threads_few_copies_std(benchmark::State& state)
{
    // NOLINTNEXTLINE
    for (auto _ : state) {
        copy_back_and_forth_between_threads(
            state.range(0), 2, 128, [](auto i) { return std::make_shared<int64_t>(i * 2); });
    }
}

// Register benchmarks

BENCHMARK(bm_copying_local)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT
BENCHMARK(bm_copying_bias)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT
BENCHMARK(bm_copying_std)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT

BENCHMARK(bm_copy_and_release_local)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT
BENCHMARK(bm_copy_and_release_bias)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT
BENCHMARK(bm_copy_and_release_std)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT

BENCHMARK(bm_copy_and_release_many_local)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT
BENCHMARK(bm_copy_and_release_many_bias)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT
BENCHMARK(bm_copy_and_release_many_std)->RangeMultiplier(2)->Range(1 << 4, 1 << 12);  // NOLINT

// local ofcourse does not work
BENCHMARK(bm_copy_back_and_forth_between_threads_many_threads_many_copies_bias)  // NOLINT
    ->RangeMultiplier(2)
    ->Range(1 << 4, 1 << 12);
BENCHMARK(bm_copy_back_and_forth_between_threads_many_threads_many_copies_std)  // NOLINT
    ->RangeMultiplier(2)
    ->Range(1 << 4, 1 << 12);

BENCHMARK(bm_copy_back_and_forth_between_threads_many_threads_few_copies_bias)  // NOLINT
    ->RangeMultiplier(2)
    ->Range(1 << 4, 1 << 12);
BENCHMARK(bm_copy_back_and_forth_between_threads_many_threads_few_copies_std)  // NOLINT
    ->RangeMultiplier(2)
    ->Range(1 << 4, 1 << 12);

BENCHMARK_MAIN();  // NOLINT
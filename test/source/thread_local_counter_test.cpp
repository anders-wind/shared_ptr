#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include <doctest/doctest.h>
#include <shared_ptr/thread_local_counter.hpp>

TEST_SUITE("thread_local_counter")
{
    TEST_CASE("No race conditions in increments to the counter")  // NOLINT
    {
        auto counter = wind::thread_local_counter {};

        auto threads = std::vector<std::thread> {};
        auto num_threads = 1 << 12;  // NOLINT
        threads.reserve(num_threads);
        for (auto i = 0; i < num_threads; i++) {
            threads.emplace_back(
                [&counter]()
                {
                    CHECK_NE(wind::cpu_id, -1);
                    ++counter;
                });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        auto sum = std::accumulate(counter.values.begin(), counter.values.end(), 0ULL);
        CHECK_EQ(sum, num_threads);
    }
}
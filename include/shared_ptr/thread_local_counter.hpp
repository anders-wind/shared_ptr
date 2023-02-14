#pragma once
#include <array>
#include <thread>

#include <pthread.h>
#include <sched.h>

namespace wind
{

thread_local const inline int64_t cpu_id = sched_getcpu();

struct thread_local_counter
{
    static_assert(WIND_NUM_LOGICAL_CORES > 1);
    std::array<std::size_t, WIND_NUM_LOGICAL_CORES> values {};

    [[nodiscard]] auto get() noexcept -> std::size_t& { return this->values.at(cpu_id); }

    [[nodiscard]] auto get() const noexcept -> const std::size_t& { return this->values.at(cpu_id); }

    auto operator++() noexcept -> std::size_t& { return ++this->get(); }
};

}  // namespace wind
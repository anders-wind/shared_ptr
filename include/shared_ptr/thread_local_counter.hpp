#pragma once
#include <array>
#include <thread>

#ifdef __linux__
#    include <sched.h>
#endif

namespace wind
{

#ifdef __linux__
thread_local const inline int64_t cpu_id = sched_getcpu();
#else
thread_local const inline int64_t cpu_id = 1;  // TODO
#endif

struct thread_local_counter
{
    static_assert(WIND_NUM_LOGICAL_CORES > 1);
    std::array<std::size_t, WIND_NUM_LOGICAL_CORES> values {};

    thread_local_counter() = default;
    explicit thread_local_counter(std::size_t val) { this->get() = val; }

    [[nodiscard]] auto get() noexcept -> std::size_t& { return this->values.at(cpu_id); }

    [[nodiscard]] auto get() const noexcept -> const std::size_t& { return this->values.at(cpu_id); }

    auto operator++() noexcept -> std::size_t& { return ++this->get(); }
    auto operator--() noexcept -> std::size_t& { return --this->get(); }
};

}  // namespace wind
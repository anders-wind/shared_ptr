#pragma once

#include <shared_ptr/thread_local_counter.hpp>

namespace wind::details
{

struct bias_ref_counter
{
    std::atomic<std::size_t> global_counter {1};
    thread_local_counter local_counter {1};

    explicit bias_ref_counter(std::size_t initial)
        : global_counter {initial}
        , local_counter {initial}
    {
    }

    auto operator++() noexcept -> std::size_t&
    {
        if (++this->local_counter == 1) {
            ++this->global_counter;
        }
        return this->local_counter.get();
    }

    [[nodiscard]] auto operator--() noexcept -> std::size_t
    {
        if (--this->local_counter == 0) {
            return --this->global_counter;
        }
        return this->local_counter.get();
    }
};

}  // namespace wind::details
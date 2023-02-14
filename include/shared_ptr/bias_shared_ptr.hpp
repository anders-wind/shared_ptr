#pragma once
#include <atomic>
#include <cassert>
#include <utility>

#include <shared_ptr/control_block.hpp>
#include <shared_ptr/thread_local_storage.hpp>

namespace wind::bias
{
template<typename T>
struct shared_ptr
{
    using counter_type = size_t;
    using local_reference_counter_type = size_t;
    using element_type = typename std::remove_extent_t<T>;
    using local_count_storage = thread_local_storage<local_reference_counter_type>;

  private:
    detail::control_block<T, std::atomic<std::size_t>>* control_block_ {nullptr};
    local_count_storage::key_t key_ {};

  public:
    shared_ptr() = default;

    explicit shared_ptr(detail::control_block_with_data<T, std::atomic<std::size_t>>* control)
        : control_block_(control)
        , key_(local_count_storage::create_key(1))
    {
    }

    explicit shared_ptr(T* data)
        : control_block_(
            detail::new_control_block_with_deleter<std::atomic<std::size_t>>(data, std::default_delete<element_type>()))
        , key_(local_count_storage::create_key(1))
    {
    }

    // stuff
    shared_ptr(const shared_ptr& other) noexcept
        : control_block_(other.control_block_)
        , key_(other.key_)
    {
        this->initial_or_inc();
    }

    shared_ptr(shared_ptr&& other) noexcept
        : control_block_(std::move(other.control_block_))
        , key_(other.key_)
    {
        other.control_block_ = nullptr;
    }

    auto operator=(const shared_ptr& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        if (this->control_block_ != other.control_block_) {
            this->decrement_and_maybe_delete();
            this->control_block_ = other.control_block_;
            this->key_ = other.key_;
            this->initial_or_inc();
        }
        return *this;
    }

    auto operator=(shared_ptr&& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        if (this->control_block_ != other.control_block_) {
            this->decrement_and_maybe_delete();
            this->control_block_ = other.control_block_;
            this->key_ = other.key_;
        } else {
            other.decrement_and_maybe_delete();
        }

        other.control_block_ = nullptr;
        return *this;
    }

    ~shared_ptr() noexcept
    {
        this->decrement_and_maybe_delete();
        this->control_block_ = nullptr;
    }

    [[nodiscard]] auto get() noexcept -> element_type*
    {
        if (this->control_block_ == nullptr) {
            return nullptr;
        }
        return this->control_block_->data;
    }

    [[nodiscard]] auto get() const noexcept -> const element_type*
    {
        if (this->control_block_ == nullptr) {
            return nullptr;
        }
        return this->control_block_->data;
    }

    [[nodiscard]] auto operator*() const -> const element_type& { return *this->control_block_->data; }

    [[nodiscard]] auto operator*() -> element_type& { return *this->control_block_->data; }

    [[nodiscard]] auto operator->() const noexcept -> const element_type*
    {
        // TODO(wind) all these if statements for getting the element should be optimized by caching T* in the
        // shared_ptr. also helps with double indirect
        if (this->control_block_ == nullptr) {
            return nullptr;
        }
        return this->control_block_->data;
    }

    [[nodiscard]] auto operator->() noexcept -> element_type*
    {
        if (this->control_block_ == nullptr) {
            return nullptr;
        }
        return this->control_block_->data;
    }

    [[nodiscard]] auto use_count() const -> const counter_type& { return this->control_block_->counter; }

    [[nodiscard]] auto unique() const -> size_t { return this->control_block_->counter == 1; }

    void swap(shared_ptr other)
    {
        std::swap(this->control_block_, other->control_block_);
        std::swap(this->key_, other->key_);
    }

    [[nodiscard]] explicit operator bool() const { return this->control_block_ != nullptr; }

  private:
    [[nodiscard]] auto get_local_counter(local_reference_counter_type initial_count = 1) noexcept
        -> local_reference_counter_type&
    {
        auto [local_counter_ref, already_existed] = local_count_storage::get_or_create(&this->key_, initial_count);
        if (!already_existed) {
            this->control_block_->inc();
        }
        return local_counter_ref.get();
    }

    void initial_or_inc() noexcept
    {
        if (this->control_block_ != nullptr) {
            this->get_local_counter(0)++;
        }
    }

    void decrement_and_maybe_delete()
    {
        if (this->control_block_ != nullptr) {
            auto& local_counter = this->get_local_counter();
            if (--local_counter == 0) {
                local_count_storage::return_key(this->key_);
                if (this->control_block_->decrement_and_check_zero()) {
                    delete this->control_block_;
                }
            }
        }
    }
};

template<typename T, typename... Args>
auto make_shared(Args&&... args) -> shared_ptr<typename std::remove_extent_t<T>>
{
    using element_type = typename std::remove_extent_t<T>;

    return shared_ptr<element_type>(
        detail::new_control_block_with_data<element_type, std::atomic<std::size_t>>(std::forward<Args>(args)...));
}

}  // namespace wind::bias

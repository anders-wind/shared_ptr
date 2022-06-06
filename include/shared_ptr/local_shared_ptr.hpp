#pragma once
#include <memory>
#include <type_traits>

#include <shared_ptr/control_block.hpp>

namespace wind::local
{
template<typename T>
struct shared_ptr
{
    using element_type = typename std::remove_extent_t<T>;
    using counter_type = size_t;

  private:
    detail::control_block<T, std::size_t>* control_block_ {nullptr};

  public:
    shared_ptr() = default;

    explicit shared_ptr(element_type* ptr)
        : control_block_(detail::new_control_block_with_deleter<std::size_t>(ptr, std::default_delete<element_type>()))
    {
    }

    template<typename DeleterF>
    shared_ptr(element_type* ptr, DeleterF&& deleter)
        : control_block_(detail::new_control_block_with_deleter<std::size_t>(ptr, std::forward<DeleterF>(deleter)))
    {
    }

    explicit shared_ptr(detail::control_block<element_type, std::size_t>* control_block)
        : control_block_(control_block)
    {
    }

    // stuff
    shared_ptr(const shared_ptr& other) noexcept
        : control_block_(other.control_block_)
    {
        this->inc();
    }

    shared_ptr(shared_ptr&& other) noexcept
        : control_block_(std::move(other.control_block_))
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
            this->inc();
        }
        return *this;
    }

    auto operator=(shared_ptr&& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        this->decrement_and_maybe_delete();
        this->control_block_ = other.control_block_;
        other.control_block_ = nullptr;
        return *this;
    }

    ~shared_ptr() noexcept
    {
        this->decrement_and_maybe_delete();
    }

    [[nodiscard]] auto get() noexcept -> element_type*
    {
        return this->control_block_->data;
    }

    [[nodiscard]] auto get() const noexcept -> const element_type*
    {
        return this->control_block_->data;
    }

    [[nodiscard]] auto operator*() const -> const element_type&
    {
        return *this->control_block_->data;
    }

    [[nodiscard]] auto operator*() -> element_type&
    {
        return *this->control_block_->data;
    }

    [[nodiscard]] auto operator->() const noexcept -> const element_type*
    {
        return this->control_block_->data;
    }

    [[nodiscard]] auto operator->() noexcept -> element_type*
    {
        return this->control_block_->data;
    }

    [[nodiscard]] auto use_count() const -> const counter_type&
    {
        return this->control_block_->counter;
    }

    [[nodiscard]] auto unique() const -> size_t
    {
        return this->control_block_->counter == 1;
    }

    void swap(shared_ptr other)
    {
        std::swap(this->control_block_, other->control_block_);
    }

    [[nodiscard]] explicit operator bool() const
    {
        return this->control_block_ != nullptr;
    }

  private:
    void inc() noexcept
    {
        if (this->control_block_ != nullptr) {
            this->control_block_->inc();
        }
    }

    void decrement_and_maybe_delete() noexcept
    {
        if (this->control_block_ != nullptr) {
            if (this->control_block_->decrement_and_check_zero()) {
                delete this->control_block_;
            }
        }
    }
};

template<typename T, typename... Args>
auto make_shared(Args&&... args) -> shared_ptr<typename std::remove_extent_t<T>>
{
    using element_type = typename std::remove_extent_t<T>;

    return shared_ptr<element_type> {
        detail::new_control_block_with_data<element_type, std::size_t>(std::forward<Args>(args)...)};
}

}  // namespace wind::local
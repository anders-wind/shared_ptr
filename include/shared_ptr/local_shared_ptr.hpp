#pragma once
#include <memory>
#include <type_traits>

namespace wind::local
{

namespace detail
{

template<typename T>
struct control_block
{
    T* data;
    size_t counter;

    control_block(T* i_data, size_t i_counter) noexcept
        : data {i_data}
        , counter {i_counter}
    {
    }

    control_block(const control_block&) noexcept = default;
    control_block(control_block&&) noexcept = default;
    auto operator=(const control_block&) noexcept -> control_block& = default;
    auto operator=(control_block&&) noexcept -> control_block& = default;

    virtual ~control_block() = default;
};

template<typename T, typename DeleterF>
struct control_block_with_deleter final : control_block<T>
{
    DeleterF deleter;

    control_block_with_deleter(T* i_data, DeleterF i_deleter) noexcept
        : control_block<T>(i_data, 1)
        , deleter {std::move(i_deleter)}
    {
    }

    control_block_with_deleter(const control_block_with_deleter&) noexcept = default;
    control_block_with_deleter(control_block_with_deleter&&) noexcept = default;
    auto operator=(const control_block_with_deleter&) noexcept -> control_block_with_deleter& = default;
    auto operator=(control_block_with_deleter&&) noexcept -> control_block_with_deleter& = default;

    ~control_block_with_deleter() noexcept override
    {
        this->deleter(this->data);
    };
};

template<typename T, typename DeleterF>
auto new_control_block_with_deleter(T* ptr, DeleterF&& deleter)
{
    return new control_block_with_deleter<T, DeleterF>(ptr, std::forward<DeleterF>(deleter));  // NOLINT
}

template<typename T>
struct control_block_with_data final : control_block<T>
{
    T val;

    template<typename... Args>
    explicit control_block_with_data(Args&&... args) noexcept
        : control_block<T>(&this->val, 1)
        , val {std::forward<Args>(args)...}
    {
    }
};

template<typename T, typename... Args>
auto new_control_block_with_data(Args&&... args)
{
    return new control_block_with_data<T>(std::forward<Args>(args)...);  // NOLINT
}

}  // namespace detail

template<typename T>
struct shared_ptr
{
    using element_type = typename std::remove_extent_t<T>;
    using counter_type = size_t;

  private:
    detail::control_block<T>* control_block_ {nullptr};

  public:
    shared_ptr() = default;

    explicit shared_ptr(element_type* ptr)
        : control_block_(detail::new_control_block_with_deleter(ptr, std::default_delete<element_type>()))
    {
    }

    template<typename DeleterF>
    shared_ptr(element_type* ptr, DeleterF&& deleter)
        : control_block_(detail::new_control_block_with_deleter<element_type>(ptr, std::forward<DeleterF>(deleter)))
    {
    }

    explicit shared_ptr(detail::control_block<element_type>* control_block)
        : control_block_(control_block)
    {
    }

    // stuff
    shared_ptr(const shared_ptr& other) noexcept
        : control_block_(other.control_block_)
    {
        this->control_block_->counter++;
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
            this->control_block_->counter++;
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
    void decrement_and_maybe_delete() noexcept
    {
        if (this->control_block_ != nullptr) {
            if (--this->control_block_->counter <= 0) {
                delete this->control_block_;
            }
        }
    }
};

template<typename T, typename... Args>
auto make_shared(Args&&... args) -> shared_ptr<typename std::remove_extent_t<T>>
{
    using element_type = typename std::remove_extent_t<T>;

    return shared_ptr<element_type> {detail::new_control_block_with_data<element_type>(std::forward<Args>(args)...)};
}

}  // namespace wind::local
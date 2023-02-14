#pragma once

namespace wind
{
namespace detail
{
template<typename T, typename CounterTypeT>
struct control_block
{
    T* data;
    CounterTypeT counter;

    explicit control_block(T* i_data) noexcept
        : data {i_data}
        , counter {1}
    {
    }

    control_block(const control_block&) noexcept = default;
    control_block(control_block&&) noexcept = default;
    auto operator=(const control_block&) noexcept -> control_block& = default;
    auto operator=(control_block&&) noexcept -> control_block& = default;

    virtual ~control_block() = default;

    void inc() noexcept { ++this->counter; }

    [[nodiscard]] auto decrement_and_check_zero() noexcept -> bool { return --this->counter == 0; }
};

template<typename T, typename CounterTypeT, typename DeleterF>
struct control_block_with_deleter final : control_block<T, CounterTypeT>
{
    DeleterF deleter;

    control_block_with_deleter(T* i_data, DeleterF i_deleter) noexcept
        : control_block<T, CounterTypeT>(i_data)
        , deleter {std::move(i_deleter)}
    {
    }

    control_block_with_deleter(const control_block_with_deleter&) noexcept = default;
    control_block_with_deleter(control_block_with_deleter&&) noexcept = default;
    auto operator=(const control_block_with_deleter&) noexcept -> control_block_with_deleter& = default;
    auto operator=(control_block_with_deleter&&) noexcept -> control_block_with_deleter& = default;

    ~control_block_with_deleter() noexcept override { this->deleter(this->data); };
};

template<typename CounterTypeT, typename T, typename DeleterF>
auto new_control_block_with_deleter(T* ptr, DeleterF&& deleter)
{
    return new control_block_with_deleter<T, CounterTypeT, DeleterF>(ptr, std::forward<DeleterF>(deleter));  // NOLINT
}

template<typename T, typename CounterTypeT>
struct control_block_with_data final : control_block<T, CounterTypeT>
{
    T val;

    template<typename... Args>
    explicit control_block_with_data(Args&&... args) noexcept
        : control_block<T, CounterTypeT>(&this->val)
        , val {std::forward<Args>(args)...}
    {
    }
};

template<typename T, typename CounterTypeT, typename... Args>
auto new_control_block_with_data(Args&&... args)
{
    return new control_block_with_data<T, CounterTypeT>(std::forward<Args>(args)...);  // NOLINT
}

}  // namespace detail
}  // namespace wind
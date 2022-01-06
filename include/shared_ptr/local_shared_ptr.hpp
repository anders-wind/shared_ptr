#pragma once

namespace wind::local
{
namespace detail
{
template<typename T>
struct t_and_count
{
    T data;
    size_t count;
};

}  // namespace detail

template<typename T>
struct shared_ptr
{
    using element_type = typename std::remove_extent_t<T>;
    using counter_type = size_t;
    using common_type = detail::t_and_count<element_type>;

  private:
    element_type* data_ {nullptr};
    counter_type* counter_ {nullptr};
    common_type* common_ptr_ {nullptr};

  public:
    shared_ptr() = default;

    explicit shared_ptr(element_type* init_data) noexcept
        : data_(init_data)
        , counter_(new counter_type(1))
        , common_ptr_(nullptr)
    {
    }

    explicit shared_ptr(common_type* common_ptr) noexcept
        : data_(&common_ptr->data)
        , counter_(&common_ptr->count)
        , common_ptr_(common_ptr)
    {
    }

    shared_ptr(const shared_ptr& other) noexcept
        : data_(other.data_)
        , counter_(other.counter_)
        , common_ptr_(other.common_ptr_)
    {
        this->get_count()++;
    }

    shared_ptr(shared_ptr&& other) noexcept
        : data_(other.data_)
        , counter_(other.counter_)
        , common_ptr_(other.common_ptr_)
    {
        other.counter_ = nullptr;
        other.data_ = nullptr;
        other.common_ptr_ = nullptr;
    }

    auto operator=(const shared_ptr& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        if (this->data_ != other.data_) {
            this->decrement_and_maybe_delete();
            this->counter_ = other.counter_;
            this->data_ = other.data_;
            this->common_ptr_ = other.common_ptr_;
            this->get_count()++;
        }
        return *this;
    }

    auto operator=(shared_ptr&& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        this->decrement_and_maybe_delete();
        this->counter_ = other.counter_;
        this->data_ = other.data_;
        this->common_ptr_ = other.common_ptr_;
        other.counter_ = nullptr;
        other.data_ = nullptr;
        other.common_ptr_ = nullptr;
        return *this;
    }

    ~shared_ptr() noexcept
    {
        this->decrement_and_maybe_delete();
    }

    [[nodiscard]] auto get() noexcept -> element_type*
    {
        return this->data_;
    }

    [[nodiscard]] auto get() const noexcept -> const element_type*
    {
        return this->data_;
    }

    [[nodiscard]] auto operator*() const -> const element_type&
    {
        return *this->data_;
    }

    [[nodiscard]] auto operator*() -> element_type&
    {
        return *this->data_;
    }

    [[nodiscard]] auto get_count() const -> const counter_type&
    {
        return *this->counter_;
    }

    [[nodiscard]] auto get_count() -> counter_type&
    {
        return *this->counter_;
    }

  private:
    void decrement_and_maybe_delete() noexcept
    {
        if (this->counter_ != nullptr) {
            if (--this->get_count() <= 0) {
                if (this->common_ptr_ == nullptr) {
                    delete this->counter_;
                    delete this->data_;
                } else {
                    delete this->common_ptr_;
                }
            }
        }
    }
};

template<typename T, typename... Args>
auto make_shared(Args&&... args) -> shared_ptr<typename std::remove_extent_t<T>>
{
    using element_type = typename std::remove_extent_t<T>;
    using s_ptr_type = shared_ptr<element_type>;
    using counter_type = typename s_ptr_type::counter_type;

    static_assert(sizeof(counter_type) == sizeof(void*));

    // NOLINTNEXTLINE
    auto memory = new detail::t_and_count<element_type> {element_type {std::forward<Args>(args)...}, size_t(1)};

    return shared_ptr<element_type>(memory);
}

}  // namespace wind::local

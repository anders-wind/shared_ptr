#pragma once

#include <atomic>
#include <string>

namespace wind
{
namespace regular
{
template<typename T>
struct shared_ptr
{
    using element_type = typename std::remove_extent_t<T>;

    element_type* data {nullptr};
    size_t* counter {nullptr};

    shared_ptr() = default;

    explicit shared_ptr(element_type* init_data)
        : data(init_data)
        , counter(new size_t(1))
    {
    }

    shared_ptr(element_type* init_data, size_t* init_counter)
        : data(init_data)
        , counter(init_counter)
    {
    }

    shared_ptr(const shared_ptr& other)
        : data(other.data)
        , counter(other.counter)
    {
        this->get_count()++;
    }

    shared_ptr(shared_ptr&& other) noexcept
        : data(other.data)
        , counter(other.counter)
    {
        other.counter = nullptr;
        other.data = nullptr;
    }

    auto operator=(const shared_ptr& other) -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        if (this->data != other.data) {
            this->decrement_and_maybe_delete();
            this->counter = other.counter;
            this->data = other.data;
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
        this->counter = other.counter;
        this->data = other.data;
        other.counter = nullptr;
        other.data = nullptr;
        return *this;
    }

    ~shared_ptr()
    {
        this->decrement_and_maybe_delete();
    }

    [[nodiscard]] auto operator*() const -> const element_type&
    {
        return *this->data;
    }

    [[nodiscard]] auto operator*() -> element_type&
    {
        return *this->data;
    }

    [[nodiscard]] auto get_count() const -> const size_t&
    {
        return *this->counter;
    }

    [[nodiscard]] auto get_count() -> size_t&
    {
        return *this->counter;
    }

  private:
    void decrement_and_maybe_delete()
    {
        if (this->counter != nullptr) {
            this->get_count()--;
            if (this->get_count() <= 0) {
                delete this->counter;
                delete this->data;
            }
        }
    }
};

template<typename T, typename... Args>
auto make_shared(Args&&... args) -> shared_ptr<typename std::remove_extent_t<T>>
{
    using element_type = typename std::remove_extent_t<T>;
    return shared_ptr<element_type>(new T {std::forward<Args>(args)...});
}

}  // namespace regular

}  // namespace wind

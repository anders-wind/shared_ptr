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

    element_type* data;
    size_t* counter;

    shared_ptr()
        : data(nullptr)
        , counter(nullptr)
    {
    }

    explicit shared_ptr(element_type* data)
        : data(data)
        , counter(new size_t(1))
    {
    }

    shared_ptr(element_type* data, size_t* counter)
        : data(data)
        , counter(counter)
    {
    }

    shared_ptr(const shared_ptr& other)
        : data(other.data)
        , counter(other.counter)
    {
        this->get_count()++;
    }

    shared_ptr(shared_ptr&& other)
        : data(other.data)
        , counter(other.counter)
    {
        other.counter = nullptr;
        other.data = nullptr;
    }

    shared_ptr& operator=(const shared_ptr& other)
    {
        if (this->data != other.data) {
            this->decrement_and_maybe_delete();
            this->counter = other.counter;
            this->data = other.data;
            this->get_count()++;
        }
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other)
    {
        this->decrement_and_maybe_delete();
        this->counter = other.counter;
        this->data = other.data;
        other.counter = nullptr;
        other.data = nullptr;
    }

    ~shared_ptr()
    {
        this->decrement_and_maybe_delete();
    }

    const auto& operator*() const
    {
        return *this->data;
    }

    auto& operator*()
    {
        return *this->data;
    }

    const size_t& get_count() const
    {
        return *this->counter;
    }

    size_t& get_count()
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
auto make_shared(Args&&... args)
{
    using element_type = typename std::remove_extent_t<T>;
    return shared_ptr<element_type>(new T(std::forward<Args>(args)...));
}

}  // namespace regular

}  // namespace wind

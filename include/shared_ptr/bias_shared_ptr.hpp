#pragma once

#include <pthread.h>  // TODO handle this cross-platform

namespace wind
{
namespace bias
{
template<typename T>
struct shared_ptr
{
    using element_type = typename std::remove_extent_t<T>;
    using local_reference_counter_type = size_t;
    using global_reference_counter_type = int16_t;

  private:
    pthread_key_t key_;
    std::atomic<global_reference_counter_type>* g_count_;
    element_type* elem_;

  public:
    shared_ptr() noexcept
        : key_ {}
        , g_count_(nullptr)
        , elem_(nullptr)
    {
    }

    explicit shared_ptr(element_type* elem) noexcept
        : key_ {}
        , g_count_(new std::atomic<global_reference_counter_type>(0))
        , elem_(elem)
    {
        pthread_key_create(&this->key_, nullptr);
        this->initialize_if_not_exists(1);
    }

    shared_ptr(const shared_ptr& other) noexcept
        : key_(other.key_)
        , g_count_(other.g_count_)
        , elem_(other.elem_)
    {
        if (this->g_count_ != nullptr) {
            this->increment_and_initialize_if_not_exists();
        }
    }

    shared_ptr& operator=(const shared_ptr& other) noexcept
    {
        this->decrement_and_maybe_delete();

        this->key_ = other.key_;
        this->elem_ = other.elem_;
        this->g_count_ = other.g_count_;

        if (this->g_count_ != nullptr) {
            this->increment_and_initialize_if_not_exists();
        }

        return *this;
    }

    // not movable for now.
    shared_ptr(shared_ptr&& other) noexcept
        : key_(std::move(other.key_))
        , g_count_(std::move(other.g_count_))
        , elem_(std::move(other.elem_))
    {
        other.g_count_ = nullptr;
        other.elem_ = nullptr;
        if (this->g_count_ != nullptr) {
            this->initialize_if_not_exists(1);
        }
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept
    {
        if (this->g_count_ != nullptr) {
            this->decrement_and_maybe_delete();
        }

        this->key_ = std::move(other.key_);
        this->elem_ = std::move(other.elem_);
        this->g_count_ = std::move(other.g_count_);

        if (this->g_count_ != nullptr) {
            this->initialize_if_not_exists(1);
        }

        return *this;
    }

    ~shared_ptr() noexcept
    {
        this->decrement_and_maybe_delete();
    }

    element_type* get() noexcept
    {
        return this->elem_;
    }

    const element_type* get() const noexcept
    {
        return this->elem_;
    }

    element_type& operator*() noexcept
    {
        return *this->elem_;
    }

    const element_type& operator*() const noexcept
    {
        return *this->elem_;
    }

    const element_type* operator->() const noexcept
    {
        return this->elem_;
    }

    element_type* operator->() noexcept
    {
        return this->elem_;
    }

    void reset(element_type* elem)
    {
        *this = shared_ptr<element_type>(elem);
    }

    void release()
    {
        this->decrement_and_maybe_delete();
    }

    operator bool() const noexcept
    {
        return this->elem_ != nullptr;
    }

    void swap(shared_ptr other)
    {
        std::swap(this->key_, other->key_);
        std::swap(this->g_count_, other->g_count_);
        std::swap(this->elem_, other->elem_);
    }

  private:
    void initialize_if_not_exists(local_reference_counter_type initial_count) noexcept
    {
        if (pthread_getspecific(this->key_) == nullptr) {
            pthread_setspecific(this->key_, new local_reference_counter_type(initial_count));
            (*this->g_count_)++;
        }
    }

    void increment_and_initialize_if_not_exists() noexcept
    {
        this->initialize_if_not_exists(0);
        (*static_cast<local_reference_counter_type*>(pthread_getspecific(this->key_)))++;
    }

    void decrement_and_maybe_delete() noexcept
    {
        auto count = static_cast<local_reference_counter_type*>(pthread_getspecific(this->key_));
        if (count != nullptr) {
            if (--(*count) <= 0) {
                if (--(*this->g_count_) <= 0) {
                    delete this->g_count_;
                    delete this->elem_;
                    pthread_key_delete(this->key_);
                }
            }
        }
    }
};

template<typename T, typename... Args>
auto make_shared(Args&&... args)
{
    using element_type = typename std::remove_extent_t<T>;
    return shared_ptr<element_type>(new T {std::forward<Args>(args)...});
}

}  // namespace bias

}  // namespace wind
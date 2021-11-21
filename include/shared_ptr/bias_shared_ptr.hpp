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

  private:
    pthread_key_t key_;
    std::atomic<int16_t>* g_count_;
    element_type* elem_;

  public:
    // shared_ptr()
    //     : g_count_(nullptr)
    //     , elem_(nullptr)
    // {
    // }

    shared_ptr(element_type* elem)
        : g_count_(new std::atomic<int16_t>(0))
        , elem_(elem)
    {
        pthread_key_create(&this->key_, nullptr);
        increment_and_initialize_if_not_exists();
    }

    shared_ptr(const shared_ptr& other)
        : key_(other.key_)
        , g_count_(other.g_count_)
        , elem_(other.elem_)
    {
        this->increment_and_initialize_if_not_exists();
    }

    shared_ptr& operator=(const shared_ptr& other)
    {
        this->decrement_and_maybe_delete();

        this->key_ = other.key_;
        this->elem_ = other.elem_;
        this->g_count_ = other.g_count_;

        this->increment_and_initialize_if_not_exists();

        return *this;
    }

    // not movable for now.
    shared_ptr(shared_ptr&& other) = delete;
    shared_ptr& operator=(shared_ptr&& other) = delete;

    ~shared_ptr()
    {
        this->decrement_and_maybe_delete();
    }

    T& get()
    {
        return *this->elem_;
    }

    const T& get() const
    {
        return *this->elem_;
    }

    operator T&()
    {
        return *this->elem_;
    }

    operator const T&() const
    {
        return *this->elem_;
    }

  private:
    void initialize_if_not_exists()
    {
        if (pthread_getspecific(this->key_) == nullptr) {
            pthread_setspecific(this->key_, new size_t(0));
            (*this->g_count_)++;
        }
    }

    void increment_and_initialize_if_not_exists()
    {
        this->initialize_if_not_exists();
        (*static_cast<size_t*>(pthread_getspecific(this->key_)))++;
    }

    void decrement_and_maybe_delete()
    {
        auto count = static_cast<size_t*>(pthread_getspecific(this->key_));
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
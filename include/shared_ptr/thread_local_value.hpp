#pragma once

#include <exception>
#include <iostream>

#include <pthread.h>  // TODO handle this cross-platform

namespace wind
{
template<typename T>
struct thread_local_value
{
  private:
    pthread_key_t key_;
    size_t* count;
    std::atomic<size_t>* g_count;

  public:
    thread_local_value()
        : g_count(new std::atomic<size_t>(0))
    {
        pthread_key_create(&this->key_, nullptr);
        increment_and_initialize_if_not_exists();
    }

    thread_local_value(const thread_local_value& other)
        : key_(other.key_)
        , count(other.count)
        , g_count(other.g_count)
    {
        this->increment_and_initialize_if_not_exists();
    }

    thread_local_value& operator=(const thread_local_value& other)
    {
        this->decrement_and_maybe_delete();

        this->key_ = other.key_;
        this->count = other.count;
        this->g_count = other.g_count;

        this->increment_and_initialize_if_not_exists();

        return *this;
    }

    // not movable for now.
    thread_local_value(thread_local_value&& other) = delete;
    thread_local_value& operator=(thread_local_value&& other) = delete;

    ~thread_local_value()
    {
        this->decrement_and_maybe_delete();
    }

    T& get()
    {
        // this->initialize_if_not_exists();
        return *static_cast<T*>(pthread_getspecific(this->key_));
    }

    operator T&()
    {
        return this->get();
    }

    const size_t& get_count() const
    {
        // this->initialize_if_not_exists();
        return *this->count;
    }

    size_t& get_count()
    {
        // this->initialize_if_not_exists();
        return *this->count;
    }

  private:
    void initialize_if_not_exists()
    {
        if (pthread_getspecific(this->key_) == nullptr) {
            pthread_setspecific(this->key_, new T());
            this->count = new size_t(0);
            (*this->g_count)++;
        }
    }

    void increment_and_initialize_if_not_exists()
    {
        initialize_if_not_exists();
        this->get_count()++;
    }

    void decrement_and_maybe_delete()
    {
        if (this->count != nullptr) {
            this->get_count()--;
            if (this->get_count() <= 0) {
                delete this->count;
                delete static_cast<T*>(pthread_getspecific(this->key_));

                (*this->g_count)--;
                if ((*this->g_count) <= 0) {
                    delete this->g_count;
                    pthread_key_delete(this->key_);
                }
            }
        }
    }
};

}  // namespace wind
#pragma once

#include <atomic>
#include <utility>

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
    pthread_key_t key_ {};
    std::atomic<global_reference_counter_type>* g_count_ {nullptr};
    element_type* elem_ {nullptr};

  public:
    shared_ptr() = default;

    explicit shared_ptr(element_type* elem) noexcept
        : g_count_(new std::atomic<global_reference_counter_type>(0))
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

    auto operator=(const shared_ptr& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        if (this->elem_ != other.elem_) {
            this->decrement_and_maybe_delete();

            this->key_ = other.key_;
            this->elem_ = other.elem_;
            this->g_count_ = other.g_count_;

            if (this->g_count_ != nullptr) {
                this->increment_and_initialize_if_not_exists();
            }
        }

        return *this;
    }

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

    auto operator=(shared_ptr&& other) noexcept -> shared_ptr&
    {
        if (this == &other) {
            return *this;
        }

        if (this->elem_ != other.elem_) {
            if (this->g_count_ != nullptr) {
                this->decrement_and_maybe_delete();
            }

            this->key_ = std::move(other.key_);
            this->elem_ = std::move(other.elem_);
            this->g_count_ = std::move(other.g_count_);

            if (this->g_count_ != nullptr) {
                this->initialize_if_not_exists(1);
            }
        }

        other.g_count_ = nullptr;
        other.elem_ = nullptr;

        return *this;
    }

    ~shared_ptr() noexcept
    {
        this->decrement_and_maybe_delete();
    }

    [[nodiscard]] auto get() noexcept -> element_type*
    {
        return this->elem_;
    }

    [[nodiscard]] auto get() const noexcept -> const element_type*
    {
        return this->elem_;
    }

    [[nodiscard]] auto operator*() noexcept -> element_type&
    {
        return *this->elem_;
    }

    [[nodiscard]] auto operator*() const noexcept -> const element_type&
    {
        return *this->elem_;
    }

    [[nodiscard]] auto operator->() const noexcept -> const element_type*
    {
        return this->elem_;
    }

    [[nodiscard]] auto operator->() noexcept -> element_type*
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

    explicit operator bool() const noexcept
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
    auto get_local_count() noexcept -> local_reference_counter_type*
    {
        return static_cast<local_reference_counter_type*>(pthread_getspecific(this->key_));
    }

    void initialize_if_not_exists(local_reference_counter_type initial_count) noexcept
    {
        if (pthread_getspecific(this->key_) == nullptr) {
            // NOLINTNEXTLINE
            pthread_setspecific(this->key_, new local_reference_counter_type(initial_count));
            this->g_count_->fetch_add(1);
        }
    }

    void increment_and_initialize_if_not_exists() noexcept
    {
        this->initialize_if_not_exists(0);
        (*this->get_local_count())++;
    }

    void decrement_and_maybe_delete() noexcept
    {
        if (this->g_count_ != nullptr) {
            auto count = this->get_local_count();
            if (count != nullptr) {
                if (--(*count) == 0) {
                    delete count;  // NOLINT(cppcoreguidelines-owning-memory)
                    // NOLINTNEXTLINE
                    pthread_setspecific(this->key_, nullptr);
                    if (this->g_count_->fetch_sub(1) - 1 == 0) {
                        delete this->g_count_;
                        delete this->elem_;
                        pthread_key_delete(this->key_);
                    }
                }
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

}  // namespace bias

}  // namespace wind
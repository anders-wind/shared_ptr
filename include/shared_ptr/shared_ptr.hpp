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

namespace biased
{
/**
 * @brief biased
 *
 * @tparam T
 */
template<typename T>
struct shared_ptr
{
  using element_type = typename std::remove_extent_t<T>;

  element_type* data;
  std::atomic<size_t>* global_counter;
  size_t* local_counter;

  shared_ptr()
      : data(nullptr)
      , global_counter(nullptr)
      , local_counter(nullptr)
  {
  }

  explicit shared_ptr(element_type* data)
      : data(data)
      , global_counter(new std::atomic<size_t>(1))
      , local_counter(new size_t(1))
  {
  }

  shared_ptr(const shared_ptr& other)
      : data(other.data)
      , global_counter(other.global_counter)
      , local_counter(other.local_counter)
  {
    this->get_local_count()++;
    // TODO(global?)
  }

  shared_ptr(shared_ptr&& other)
      : data(other.data)
      , global_counter(other.global_counter)
      , local_counter(other.local_counter)
  {
    other.data = nullptr;
    other.global_counter = nullptr;
    other.local_counter = nullptr;
  }

  shared_ptr& operator=(const shared_ptr& other)
  {
    if (this->data != other.data) {
      this->decrement_and_maybe_delete();
      this->local_counter = other.local_counter;
      this->global_counter = other.global_counter;
      this->data = other.data;
      this->get_local_count()++;
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

  const size_t& get_local_count() const
  {
    return *this->local_counter;
  }

  size_t& get_local_count()
  {
    return *this->local_counter;
  }

  const std::atomic<size_t>& get_global_count() const
  {
    return *this->global_counter;
  }

  std::atomic<size_t>& get_global_count()
  {
    return *this->global_counter;
  }

private:
  void decrement_and_maybe_delete()
  {
    if (this->local_counter != nullptr) {
      this->get_local_count()--;
      if (this->get_local_count() <= 0) {
        this->get_global_count()--;
        if (this->get_global_count() == 0) {
          delete this->local_counter;
          delete this->data;
          delete this->global_counter;
        }
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

}  // namespace biased

}  // namespace wind

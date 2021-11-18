#pragma once

#include <string>

namespace wind
{
template<typename T>
struct SharedPtr
{
  using element_type = typename std::remove_extent_t<T>;

  element_type* data;
  size_t* counter;

  SharedPtr()
      : data(nullptr)
      , counter(nullptr)
  {
  }

  explicit SharedPtr(element_type* data)
      : data(data)
      , counter(new size_t(1))
  {
  }

  SharedPtr(element_type* data, size_t* counter)
      : data(data)
      , counter(counter)
  {
  }

  SharedPtr(const SharedPtr& other)
      : data(other.data)
      , counter(other.counter)
  {
    this->get_count()++;
  }

  SharedPtr(SharedPtr&& other)
      : data(other.data)
      , counter(other.counter)
  {
    other.counter = nullptr;
    other.data = nullptr;
  }

  SharedPtr& operator=(const SharedPtr& other)
  {
    if (this->data != other.data) {
      this->decrement_and_maybe_delete();
      this->counter = other.counter;
      this->data = other.data;
      this->get_count()++;
    }
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& other)
  {
    this->decrement_and_maybe_delete();
    this->counter = other.counter;
    this->data = other.data;
    other.counter = nullptr;
    other.data = nullptr;
  }

  ~SharedPtr()
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
  return SharedPtr<element_type>(new T(std::forward<Args>(args)...));
}

}  // namespace wind

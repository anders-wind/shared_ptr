#include <cinttypes>

#include <robin_hood.h>

namespace wind
{
template<typename T>
struct thread_local_storage
{
    using key_t = std::size_t;

    static constexpr std::size_t initial_storage = 1024;

  private:
    static auto values() -> robin_hood::unordered_map<key_t, T>&
    {
        thread_local robin_hood::unordered_map<key_t, T> values = robin_hood::unordered_map<key_t, T>(initial_storage);
        return values;
    }

  public:
    static auto create_key(T initial_val) -> key_t
    {
        auto key = key_t {values().size()};
        values().emplace(key, initial_val);
        return key;
    }

    static auto get_or_create(key_t* key, T initial_val) -> std::tuple<std::reference_wrapper<T>, bool>
    {
        if (auto it = values().find(*key); it != values().end()) {
            return {it->second, true};
        }
        *key = create_key(initial_val);
        return {values().at(*key), false};
    }

    static auto contains(key_t key) -> bool
    {
        return values().contains(key);
    }

    static auto get(key_t key) -> T&
    {
        return values().at(key);
    }
    static auto return_key(key_t key) -> void
    {
        values().erase(key);
    }
};

}  // namespace wind

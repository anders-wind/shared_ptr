#include <functional>
#include <vector>

#include <doctest/doctest.h>
#include <shared_ptr/local_shared_ptr.hpp>

TEST_SUITE("local::shared_ptr")  // NOLINT
{
    TEST_CASE("local::shared_ptr: make_shared works")  // NOLINT
    {
        auto my_shared = wind::local::make_shared<int>(42);
        CHECK(*my_shared == 42);
        CHECK(my_shared.use_count() == 1);
    }

    TEST_CASE("local::shared_ptr: copy operator= works")  // NOLINT
    {
        auto my_shared = wind::local::make_shared<int>(42);
        auto copy = my_shared;
        CHECK((*copy) == (*my_shared));
        CHECK(my_shared.use_count() == 2);
        CHECK(copy.use_count() == 2);
    }

    TEST_CASE("local::shared_ptr: move operator= works")  // NOLINT
    {
        auto my_shared = wind::local::make_shared<int>(42);
        auto moved = std::move(my_shared);
        CHECK(moved.use_count() == 1);
    }

    TEST_CASE("local::shared_ptr: Destructor does not delete when copy exists")  // NOLINT
    {
        wind::local::shared_ptr<int> out_copy;
        {
            auto my_shared = wind::local::make_shared<int>(42);
            out_copy = my_shared;
            CHECK(out_copy.use_count() == 2);
        }
        CHECK(out_copy.use_count() == 1);
        CHECK(*out_copy == 42);
    }

    struct deleter_func_2  // NOLINT
    {
        std::function<void()> delete_func;
        explicit deleter_func_2(std::function<void()> func)
            : delete_func(std::move(func))
        {
        }
        ~deleter_func_2()
        {
            this->delete_func();
        }
    };

    TEST_CASE("local::shared_ptr: Delete gets called")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr = wind::local::make_shared<deleter_func_2>([&was_called]() { was_called = true; });
            CHECK(!was_called);
        }
        CHECK(was_called);
    }

    TEST_CASE("local::shared_ptr: Delete gets called when supplying pointer")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr =
                wind::local::shared_ptr<deleter_func_2>(new deleter_func_2([&was_called]() { was_called = true; }));
            CHECK(!was_called);
        }
        CHECK(was_called);
    }

    TEST_CASE("local::shared_ptr: Delete gets called when 2 copies")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr = wind::local::make_shared<deleter_func_2>([&was_called]() { was_called = true; });
            auto copy = ptr;  // NOLINT
            CHECK(!was_called);
        }
        CHECK(was_called);
    }

    TEST_CASE("local::shared_ptr: Delete only gets called on last destructor")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr = wind::local::make_shared<deleter_func_2>([&was_called]() { was_called = true; });
            {
                auto copy = ptr;  // NOLINT
            }
            CHECK(!was_called);
        }
        CHECK(was_called);
    }

    TEST_CASE("local::shared_ptr: copying a nullptr is okay")
    {
        auto empty = wind::local::shared_ptr<int>();
        auto copy = empty;  // NOLINT
    }

    TEST_CASE("local::shared_ptr: push_back on vector")
    {
        auto ptrs = std::vector<wind::local::shared_ptr<int>>();
        for (int i = 0; i < static_cast<int>(1024); i++) {
            ptrs.push_back(wind::local::make_shared<int>(42 * i));
        }
    }
}
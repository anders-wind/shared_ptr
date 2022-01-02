#include <functional>

#include <doctest/doctest.h>
#include <shared_ptr/local_shared_ptr.hpp>

TEST_SUITE("regular::shared_ptr")  // NOLINT
{
    TEST_CASE("regular::shared_ptr: make_shared works")  // NOLINT
    {
        auto my_shared = wind::regular::make_shared<int>(42);
        CHECK(*my_shared == 42);
        CHECK(my_shared.get_count() == 1);
    }

    TEST_CASE("regular::shared_ptr: copy operator= works")  // NOLINT
    {
        auto my_shared = wind::regular::make_shared<int>(42);
        auto copy = my_shared;
        CHECK((*copy) == (*my_shared));
        CHECK(my_shared.get_count() == 2);
        CHECK(copy.get_count() == 2);
    }

    TEST_CASE("regular::shared_ptr: move operator= works")  // NOLINT
    {
        auto my_shared = wind::regular::make_shared<int>(42);
        auto moved = std::move(my_shared);
        CHECK(moved.get_count() == 1);
    }

    TEST_CASE("regular::shared_ptr: Destructor does not delete when copy exists")  // NOLINT
    {
        wind::regular::shared_ptr<int> out_copy;
        {
            auto my_shared = wind::regular::make_shared<int>(42);
            out_copy = my_shared;
            CHECK(out_copy.get_count() == 2);
        }
        CHECK(out_copy.get_count() == 1);
        CHECK(*out_copy.data == 42);
    }

    struct deleter_func  // NOLINT
    {
        std::function<void()> delete_func;
        ~deleter_func()
        {
            delete_func();
        }
    };

    TEST_CASE("regular::shared_ptr: Delete gets called")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr =
                wind::regular::make_shared<deleter_func>([&was_called]() { was_called = true; });
        }
        CHECK(was_called);
    }

    TEST_CASE("regular::shared_ptr: Delete gets called when 2 copies")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr =
                wind::regular::make_shared<deleter_func>([&was_called]() { was_called = true; });
            auto copy = ptr;  // NOLINT
        }
        CHECK(was_called);
    }

    TEST_CASE("regular::shared_ptr: Delete only gets called on last destructor")  // NOLINT
    {
        bool was_called = false;
        {
            auto ptr =
                wind::regular::make_shared<deleter_func>([&was_called]() { was_called = true; });
            {
                auto copy = ptr;  // NOLINT
            }
            CHECK(!was_called);
        }
        CHECK(was_called);
    }
}
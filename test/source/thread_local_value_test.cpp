
#include <thread>

#include <doctest/doctest.h>
#include <shared_ptr/thread_local_value.hpp>

TEST_SUITE("thread_local_value")
{
    TEST_CASE("thread_local_value: can store stuff")
    {
        auto value = wind::thread_local_value<int>();
        CHECK(value.get() == 0);
        value.get()++;
        CHECK(value.get() == 1);
    }

    TEST_CASE("thread_local_value: two values do not share state")
    {
        auto value1 = wind::thread_local_value<int>();
        value1.get() = 1;
        auto value2 = wind::thread_local_value<int>();
        value2.get() = 42;
        CHECK(value1 == 1);
        CHECK(value2 == 42);
    }

    TEST_CASE("thread_local_value: copies work")
    {
        auto value1 = wind::thread_local_value<int>();
        value1.get() = 1;
        auto copy = value1;
        CHECK(value1 == 1);
        CHECK(copy == 1);
    }

    TEST_CASE("thread_local_value: copies and modifications to it 'work")
    {
        auto value1 = wind::thread_local_value<int>();
        value1.get() = 1;
        auto copy = value1;
        copy.get()++;
        CHECK(value1 == 2);
        CHECK(copy == 2);
    }

    TEST_CASE("thread_local_value: work on one thread does not move to the other")
    {
        auto value1 = wind::thread_local_value<int>();
        value1.get() = 1;

        auto thread_work = std::thread(
            [&value1]()
            {
                auto local_copy = value1;
                CHECK(local_copy.get() == 0);
                local_copy.get() = 42;
                CHECK(local_copy.get() == 42);
            });
        thread_work.join();
        CHECK(value1.get() == 1);
    }

    struct DeleterFunc
    {
        bool* was_deleted;
        ~DeleterFunc()
        {
            *was_deleted = true;
        }
    };

    TEST_CASE("thread_local_value: destructor gets")
    {
        auto was_called = std::make_unique<bool>(false);
        {
            auto value = wind::thread_local_value<DeleterFunc>();
            value.get().was_deleted = was_called.get();
            {
                auto copy = value;
            }
            CHECK(!*value.get().was_deleted);
        }
        CHECK(*was_called.get());
    }
}
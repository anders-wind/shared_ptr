
#include <thread>

#include <doctest/doctest.h>
#include <shared_ptr/bias_shared_ptr.hpp>

TEST_SUITE("thread_local_value")
{
    TEST_CASE("thread_local_value: can store stuff")
    {
        auto value = wind::bias::make_shared<int>(0);
        CHECK(value.get() == 0);
        value.get()++;
        CHECK(value.get() == 1);
    }

    TEST_CASE("thread_local_value: two values do not share state")
    {
        auto value1 = wind::bias::make_shared<int>(1);
        value1.get()++;
        auto value2 = wind::bias::make_shared<int>(42);
        CHECK(value1 == 2);
        CHECK(value2 == 42);
    }

    TEST_CASE("thread_local_value: copies work")
    {
        auto value1 = wind::bias::make_shared<int>();
        value1.get() = 1;
        auto copy = value1;
        CHECK(value1 == 1);
        CHECK(copy == 1);
    }

    TEST_CASE("thread_local_value: copies and modifications to it 'work")
    {
        auto value1 = wind::bias::make_shared<int>();
        value1.get() = 1;
        auto copy = value1;
        copy.get()++;
        CHECK(value1 == 2);
        CHECK(copy == 2);
    }

    TEST_CASE("thread_local_value: assigning values on another thread works")
    {
        auto value1 = wind::bias::make_shared<int>();
        value1.get() = 1;

        auto thread_work = std::thread(
            [&value1]()
            {
                auto local_copy = value1;
                CHECK(local_copy.get() == 1);
                local_copy.get() = 42;
                CHECK(local_copy.get() == 42);
                // local thread copy does not delete
            });
        thread_work.join();
        CHECK(value1.get() == 42);
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
            auto value = wind::bias::make_shared<DeleterFunc>();
            value.get().was_deleted = was_called.get();
            {
                auto copy = value;
            }
            CHECK(!*value.get().was_deleted);
        }
        CHECK(*was_called.get());
    }
}
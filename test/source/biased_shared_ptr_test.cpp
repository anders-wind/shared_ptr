
#include <thread>
#include <vector>

#include <doctest/doctest.h>
#include <shared_ptr/bias_shared_ptr.hpp>

TEST_SUITE("bias::shared_ptr")  // NOLINT
{
    TEST_CASE("bias::shared_ptr: nullptr destruction is ok for copies")  // NOLINT
    {
        wind::bias::shared_ptr<int> val;

        auto copy = val;  // NOLINT

        wind::bias::shared_ptr<int> operator_copy;
        operator_copy = copy;

        auto init_operator_copy = wind::bias::make_shared<int>(42);
        init_operator_copy = operator_copy;
    }

    TEST_CASE("bias::shared_ptr: nullptr destruction is ok for moves")  // NOLINT
    {
        wind::bias::shared_ptr<int> val;

        auto copy = std::move(val);

        wind::bias::shared_ptr<int> operator_move;
        operator_move = std::move(copy);

        auto init_operator_move = wind::bias::make_shared<int>(42);
        init_operator_move = std::move(operator_move);
    }

    TEST_CASE("bias::shared_ptr: copy constructors work")  // NOLINT
    {
        auto val = wind::bias::make_shared<int>(42);

        // copy constructor
        auto copy = val;
        CHECK(*val == 42);
        CHECK(*copy == 42);

        // copy operator= with uninitialized
        wind::bias::shared_ptr<int> operator_copy;
        operator_copy = copy;
        CHECK(*copy == 42);
        CHECK(*operator_copy == 42);

        // copy operator= with initialized
        auto init_operator_copy = wind::bias::make_shared<int>(3);
        init_operator_copy = operator_copy;
        CHECK(*operator_copy == 42);
        CHECK(*init_operator_copy == 42);
    }

    TEST_CASE("bias::shared_ptr: move constructors work")  // NOLINT
    {
        auto val = wind::bias::make_shared<int>(42);

        // move constructor;
        auto copy = std::move(val);
        CHECK(*copy == 42);

        // move operator= with uninitialized;
        wind::bias::shared_ptr<int> operator_copy;
        operator_copy = std::move(copy);
        CHECK(*operator_copy == 42);

        // move operator= with initialized;
        auto init_operator_copy = wind::bias::make_shared<int>(3);
        init_operator_copy = std::move(operator_copy);
        CHECK(*init_operator_copy == 42);
    }

    TEST_CASE("bias::shared_ptr: can store stuff")  // NOLINT
    {
        auto value = wind::bias::make_shared<int>(0);
        CHECK(*value == 0);
        (*value)++;
        CHECK(*value == 1);
    }

    TEST_CASE("bias::shared_ptr: two values do not share state")  // NOLINT
    {
        auto value1 = wind::bias::make_shared<int>(1);
        (*value1)++;
        auto value2 = wind::bias::make_shared<int>(42);
        CHECK(*value1 == 2);
        CHECK(*value2 == 42);
    }

    TEST_CASE("bias::shared_ptr: copies work")  // NOLINT
    {
        auto value1 = wind::bias::make_shared<int>();
        *value1 = 1;
        auto copy = value1;
        CHECK(*value1 == 1);
        CHECK(*copy == 1);
    }

    TEST_CASE("bias::shared_ptr: copies and modifications to it 'work")  // NOLINT
    {
        auto value1 = wind::bias::make_shared<int>();
        *value1 = 1;
        auto copy = value1;
        (*copy)++;
        CHECK(*value1 == 2);
        CHECK(*copy == 2);
    }

    TEST_CASE("bias::shared_ptr: assigning values on another thread works")  // NOLINT
    {
        auto value1 = wind::bias::make_shared<int>();
        *value1 = 1;

        auto thread1 = std::thread(
            [&value1]()
            {
                auto local_copy = value1;
                CHECK(*local_copy == 1);
                *local_copy = 42;
                CHECK(*local_copy == 42);
                // local thread copy does not delete
            });
        thread1.join();
        CHECK(*value1 == 42);

        auto thread2 = std::thread(
            [&value1]()
            {
                *value1 = 3;
                CHECK(*value1 == 3);
            });
        thread2.join();
        CHECK(*value1 == 3);
    }

    struct deleter_func  // NOLINT
    {
        bool* was_deleted;
        ~deleter_func()
        {
            *was_deleted = true;
        }
    };

    TEST_CASE("bias::shared_ptr: destructor gets")  // NOLINT
    {
        auto was_called = std::make_unique<bool>(false);
        {
            auto value = wind::bias::make_shared<deleter_func>();

            value->was_deleted = was_called.get();
            {
                auto copy = value;  // NOLINT
            }
            CHECK(!(*value->was_deleted));
        }
        CHECK(*was_called.get());
    }

    TEST_CASE("bias::shared_ptr: can put into vector")  // NOLINT
    {
        auto ptrs = std::vector<wind::bias::shared_ptr<int>>();
        for (auto i = 0; i < 2; i++) {
            ptrs.push_back(wind::bias::make_shared<int>(i + 1));
        }
        CHECK(*ptrs[0] == 1);
        CHECK(*ptrs[1] == 2);

        auto default_ptrs = std::vector<wind::bias::shared_ptr<int>>(2);
        CHECK(default_ptrs[0].get() == nullptr);
        CHECK(default_ptrs[1].get() == nullptr);
    }

    TEST_CASE("bias::shared_ptr: operator= self is ok")  // NOLINT
    {
        auto ptr = wind::bias::make_shared<int>(3);
        auto copy = ptr;
        auto copy2 = ptr;
        copy = copy2;
        CHECK(*ptr == 3);
    }
}
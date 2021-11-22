
#include <thread>
#include <vector>

#include <doctest/doctest.h>
#include <shared_ptr/bias_shared_ptr.hpp>

TEST_SUITE("bias::shared_ptr")
{
    TEST_CASE("bias::shared_ptr: nullptr destruction is ok for copies")
    {
        wind::bias::shared_ptr<int> val;

        auto copy = val;

        wind::bias::shared_ptr<int> operator_copy;
        operator_copy = copy;

        auto init_operator_copy = wind::bias::make_shared<int>(42);
        init_operator_copy = operator_copy;
    }

    TEST_CASE("bias::shared_ptr: nullptr destruction is ok for moves")
    {
        wind::bias::shared_ptr<int> val;

        auto copy = std::move(val);

        wind::bias::shared_ptr<int> operator_move;
        operator_move = std::move(copy);

        auto init_operator_move = wind::bias::make_shared<int>(42);
        init_operator_move = std::move(operator_move);
    }

    TEST_CASE("bias::shared_ptr: copy constructors work")
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
        auto init_operator_copy = wind::bias::make_shared<int>(99);
        init_operator_copy = operator_copy;
        CHECK(*operator_copy == 42);
        CHECK(*init_operator_copy == 42);
    }

    TEST_CASE("bias::shared_ptr: move constructors work")
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
        auto init_operator_copy = wind::bias::make_shared<int>(99);
        init_operator_copy = std::move(operator_copy);
        CHECK(*init_operator_copy == 42);
    }

    TEST_CASE("bias::shared_ptr: can store stuff")
    {
        auto value = wind::bias::make_shared<int>(0);
        CHECK(*value == 0);
        (*value)++;
        CHECK(*value == 1);
    }

    TEST_CASE("bias::shared_ptr: two values do not share state")
    {
        auto value1 = wind::bias::make_shared<int>(1);
        (*value1)++;
        auto value2 = wind::bias::make_shared<int>(42);
        CHECK(*value1 == 2);
        CHECK(*value2 == 42);
    }

    TEST_CASE("bias::shared_ptr: copies work")
    {
        auto value1 = wind::bias::make_shared<int>();
        *value1 = 1;
        auto copy = value1;
        CHECK(*value1 == 1);
        CHECK(*copy == 1);
    }

    TEST_CASE("bias::shared_ptr: copies and modifications to it 'work")
    {
        auto value1 = wind::bias::make_shared<int>();
        *value1 = 1;
        auto copy = value1;
        (*copy)++;
        CHECK(*value1 == 2);
        CHECK(*copy == 2);
    }

    TEST_CASE("bias::shared_ptr: assigning values on another thread works")
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
                *value1 = 99;
                CHECK(*value1 == 99);
            });
        thread2.join();
        CHECK(*value1 == 99);
    }

    struct DeleterFunc
    {
        bool* was_deleted;
        ~DeleterFunc()
        {
            *was_deleted = true;
        }
    };

    TEST_CASE("bias::shared_ptr: destructor gets")
    {
        auto was_called = std::make_unique<bool>(false);
        {
            auto value = wind::bias::make_shared<DeleterFunc>();

            value->was_deleted = was_called.get();
            {
                auto copy = value;
            }
            CHECK(!(*value->was_deleted));
        }
        CHECK(*was_called.get());
    }

    TEST_CASE("bias::shared_ptr: can put into vector")
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

    TEST_CASE("bias::shared_ptr: operator= self is ok")
    {
        auto ptr = wind::bias::make_shared<int>(5);
        auto copy = ptr;
        auto copy2 = ptr;
        copy = copy2;
        CHECK(*ptr == 5);
    }
}

#include <thread>
#include <vector>

#include <doctest/doctest.h>
#include <shared_ptr/bias_shared_ptr.hpp>
#include <shared_ptr/bias_shared_ptr_2.hpp>

TEST_SUITE("bias::shared_ptr")  // NOLINT
{
    TEST_CASE("bias::shared_ptr: nullptr destruction is ok for copies")  // NOLINT
    {
        wind::bias_2::shared_ptr<int> val;

        auto copy = val;  // NOLINT

        wind::bias_2::shared_ptr<int> operator_copy;
        operator_copy = copy;

        auto init_operator_copy = wind::bias_2::make_shared<int>(42);
        init_operator_copy = operator_copy;
    }

    TEST_CASE("bias::shared_ptr: nullptr destruction is ok for moves")  // NOLINT
    {
        wind::bias_2::shared_ptr<int> val;

        auto copy = std::move(val);

        wind::bias_2::shared_ptr<int> operator_move;
        operator_move = std::move(copy);

        auto init_operator_move = wind::bias_2::make_shared<int>(42);
        init_operator_move = std::move(operator_move);
    }

    TEST_CASE("bias::shared_ptr: copy constructors work")  // NOLINT
    {
        auto val = wind::bias_2::make_shared<int>(42);

        // copy constructor
        auto copy = val;
        CHECK(*val == 42);
        CHECK(*copy == 42);

        // copy operator= with uninitialized
        wind::bias_2::shared_ptr<int> operator_copy;
        operator_copy = copy;
        CHECK(*copy == 42);
        CHECK(*operator_copy == 42);

        // copy operator= with initialized
        auto init_operator_copy = wind::bias_2::make_shared<int>(3);
        init_operator_copy = operator_copy;
        CHECK(*operator_copy == 42);
        CHECK(*init_operator_copy == 42);
    }

    TEST_CASE("bias::shared_ptr: move constructors work")  // NOLINT
    {
        auto val = wind::bias_2::make_shared<int>(42);

        // move constructor;
        auto copy = std::move(val);
        CHECK(*copy == 42);

        // move operator= with uninitialized;
        wind::bias_2::shared_ptr<int> operator_copy;
        operator_copy = std::move(copy);
        CHECK(*operator_copy == 42);

        // move operator= with initialized;
        auto init_operator_copy = wind::bias_2::make_shared<int>(3);
        init_operator_copy = std::move(operator_copy);
        CHECK(*init_operator_copy == 42);
    }

    TEST_CASE("bias::shared_ptr: can store stuff")  // NOLINT
    {
        auto value = wind::bias_2::make_shared<int>(0);
        CHECK(*value == 0);
        (*value)++;
        CHECK(*value == 1);
    }

    TEST_CASE("bias::shared_ptr: two values do not share state")  // NOLINT
    {
        auto value1 = wind::bias_2::make_shared<int>(1);
        (*value1)++;
        auto value2 = wind::bias_2::make_shared<int>(42);
        CHECK(*value1 == 2);
        CHECK(*value2 == 42);
    }

    TEST_CASE("bias::shared_ptr: copies work")  // NOLINT
    {
        auto value1 = wind::bias_2::make_shared<int>();
        *value1 = 1;
        auto copy = value1;
        CHECK(*value1 == 1);
        CHECK(*copy == 1);
    }

    TEST_CASE("bias::shared_ptr: copies and modifications to it 'work")  // NOLINT
    {
        auto value1 = wind::bias_2::make_shared<int>();
        *value1 = 1;
        auto copy = value1;
        (*copy)++;
        CHECK(*value1 == 2);
        CHECK(*copy == 2);
    }

    TEST_CASE("bias::shared_ptr: assigning values on another thread works")  // NOLINT
    {
        auto value1 = wind::bias_2::make_shared<int>();
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

    // NOLINTNEXTLINE
    struct deleter_ref
    {
        bool* was_deleted {nullptr};
        ~deleter_ref()
        {
            *this->was_deleted = true;
        }
    };

    TEST_CASE("bias::shared_ptr: destructor gets")  // NOLINT
    {
        auto was_called = false;
        {
            auto value = wind::bias_2::make_shared<deleter_ref>();

            value->was_deleted = &was_called;
            {
                auto copy = value;  // NOLINT
            }
            CHECK(!(*value->was_deleted));
        }
        CHECK(was_called);
    }

    TEST_CASE("bias::shared_ptr: can put into vector")  // NOLINT
    {
        auto ptrs = std::vector<wind::bias_2::shared_ptr<int>>();
        for (auto i = 0; i < 2; i++) {
            ptrs.push_back(wind::bias_2::make_shared<int>(i + 1));
        }
        CHECK(*ptrs[0] == 1);
        CHECK(*ptrs[1] == 2);

        auto default_ptrs = std::vector<wind::bias_2::shared_ptr<int>>(2);
        CHECK(default_ptrs[0].get() == nullptr);
        CHECK(default_ptrs[1].get() == nullptr);
    }

    TEST_CASE("bias::shared_ptr: operator= self is ok")  // NOLINT
    {
        auto ptr = wind::bias_2::make_shared<int>(3);
        auto copy = ptr;
        auto copy2 = ptr;
        copy = copy2;
        CHECK(*ptr == 3);
    }

    TEST_CASE("bias::shared_ptr: copying a nullptr is okay")
    {
        auto empty = wind::bias_2::shared_ptr<int>();
        auto copy = empty;  // NOLINT
    }

    // Fails because GCC pthreads can only handle 1024 elements in pthread_key storage
    // TEST_CASE("bias::shared_ptr: push_back on vector")
    // {
    //     {
    //         auto ptrs = std::vector<wind::bias_2::shared_ptr<int>>();
    //         const auto number_of_push_backs_until_resize = 1024;
    //         for (int64_t i = 0; i < number_of_push_backs_until_resize - 1; i++) {
    //             ptrs.push_back(wind::bias_2::make_shared<int>(static_cast<int>(42 * i)));
    //         }
    //         ptrs.push_back(wind::bias_2::make_shared<int>(static_cast<int>(-2)));
    //         CHECK(ptrs.size() == number_of_push_backs_until_resize);
    //     }
    // }

    TEST_CASE("bias::shared_ptr: push_back same elem on vector")
    {
        auto ptr = wind::bias_2::make_shared<int>(static_cast<int>(42));
        auto ptrs = std::vector<wind::bias_2::shared_ptr<int>>();
        const auto number_of_push_backs_until_resize = 1024;
        for (int64_t i = 0; i < number_of_push_backs_until_resize - 1; i++) {
            ptrs.push_back(ptr);
        }
        ptrs.push_back(ptr);
        CHECK(ptrs.size() == number_of_push_backs_until_resize);
    }
}
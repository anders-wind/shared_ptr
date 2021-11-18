#include "shared_ptr/shared_ptr.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("shared_ptr")
{
  TEST_CASE("make_shared works")
  {
    auto my_shared = wind::make_shared<int>(42);
    CHECK(*my_shared == 42);
    CHECK(*my_shared.counter == 1);
  }

  TEST_CASE("copy operator= works")
  {
    auto my_shared = wind::make_shared<int>(42);
    auto copy = my_shared;
    CHECK((*copy) == (*my_shared));
    CHECK(my_shared.get_count() == 2);
    CHECK(copy.get_count() == 2);
  }

  TEST_CASE("move operator= works")
  {
    auto my_shared = wind::make_shared<int>(42);
    auto moved = std::move(my_shared);
    CHECK(moved.get_count() == 1);
  }

  TEST_CASE("Destructor does not delete when copy exists")
  {
    wind::SharedPtr<int> out_copy;
    {
      auto my_shared = wind::make_shared<int>(42);
      out_copy = my_shared;
      CHECK(out_copy.get_count() == 2);
    }
    CHECK(out_copy.get_count() == 1);
    CHECK(*out_copy.data == 42);
  }
}

#include "shared_ptr/shared_ptr.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("shared_ptr")
{
  TEST_CASE("Beginning")
  {
    CHECK(1 == 1);
  }
}
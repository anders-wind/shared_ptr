#include "shared_ptr/shared_ptr.hpp"

auto main() -> int
{
  auto result = name();
  return result == "shared_ptr" ? 0 : 1;
}

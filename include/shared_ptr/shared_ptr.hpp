#pragma once

#include <string>

namespace wind
{
/**
 * @brief Return the name of this header-only library
 */
inline auto name() -> std::string
{
  return "shared_ptr";
}

}  // namespace wind

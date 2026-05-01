/**
 *
 *  @file Result.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/sdk-cpp
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra C++ SDK
 *
 */

#ifndef SOFTADASTRA_SDK_RESULT_HPP
#define SOFTADASTRA_SDK_RESULT_HPP

#include <softadastra/core/types/Result.hpp>

namespace softadastra::sdk
{
  /**
   * @brief Public SDK result type.
   *
   * Result<T, E> represents either a successful value of type T or an error
   * of type E.
   *
   * It is intentionally mapped to the stable Softadastra core Result type so
   * the SDK does not duplicate error-handling primitives.
   *
   * Typical usage:
   *
   * @code
   * softadastra::sdk::Result<std::string, Error> result = client.get("key");
   *
   * if (result.is_ok())
   * {
   *   auto value = result.value();
   * }
   * else
   * {
   *   auto error = result.error();
   * }
   * @endcode
   */
  template <typename T, typename E>
  using Result = softadastra::core::types::Result<T, E>;

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_RESULT_HPP

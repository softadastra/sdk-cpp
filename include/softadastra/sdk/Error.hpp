/**
 *
 *  @file Error.hpp
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

#ifndef SOFTADASTRA_SDK_ERROR_HPP
#define SOFTADASTRA_SDK_ERROR_HPP

#include <string>
#include <string_view>
#include <utility>

#include <softadastra/core/errors/Error.hpp>
#include <softadastra/core/errors/ErrorCode.hpp>
#include <softadastra/core/errors/Severity.hpp>

namespace softadastra::sdk
{
  namespace core_errors = softadastra::core::errors;

  /**
   * @brief Public SDK error object.
   *
   * Error is the developer-facing error type returned by SDK operations.
   *
   * It wraps the stable Softadastra core error model while keeping the SDK API
   * small, clear, and independent from internal module details.
   */
  class Error
  {
  public:
    /**
     * @brief Creates an empty success-like error.
     */
    Error() = default;

    /**
     * @brief Creates an SDK error from core fields.
     *
     * @param code Machine-readable error code.
     * @param message Developer-facing diagnostic message.
     * @param severity Error severity.
     */
    Error(
        core_errors::ErrorCode code,
        std::string message,
        core_errors::Severity severity = core_errors::Severity::Error)
        : code_(code),
          severity_(severity),
          message_(std::move(message))
    {
    }

    /**
     * @brief Creates an SDK error from a core Softadastra error.
     *
     * @param error Core Softadastra error.
     */
    explicit Error(const core_errors::Error &error)
        : code_(error.code()),
          severity_(error.severity()),
          message_(error.message())
    {
      if (!error.context().empty())
      {
        context_ = error.context().message();
      }
    }

    /**
     * @brief Creates a generic SDK error.
     *
     * @param message Developer-facing diagnostic message.
     * @return SDK error.
     */
    [[nodiscard]] static Error make(std::string message)
    {
      return Error{
          core_errors::ErrorCode::Unknown,
          std::move(message),
          core_errors::Severity::Error};
    }

    /**
     * @brief Creates an SDK error from a core Softadastra error.
     *
     * @param error Core Softadastra error.
     * @return SDK error.
     */
    [[nodiscard]] static Error from_core(const core_errors::Error &error)
    {
      return Error{error};
    }

    /**
     * @brief Creates an invalid argument error.
     *
     * @param message Developer-facing diagnostic message.
     * @return SDK error.
     */
    [[nodiscard]] static Error invalid_argument(std::string message)
    {
      return Error{
          core_errors::ErrorCode::InvalidArgument,
          std::move(message),
          core_errors::Severity::Error};
    }

    /**
     * @brief Creates an invalid state error.
     *
     * @param message Developer-facing diagnostic message.
     * @return SDK error.
     */
    [[nodiscard]] static Error invalid_state(std::string message)
    {
      return Error{
          core_errors::ErrorCode::InvalidState,
          std::move(message),
          core_errors::Severity::Error};
    }

    /**
     * @brief Creates a not found error.
     *
     * @param message Developer-facing diagnostic message.
     * @return SDK error.
     */
    [[nodiscard]] static Error not_found(std::string message)
    {
      return Error{
          core_errors::ErrorCode::NotFound,
          std::move(message),
          core_errors::Severity::Error};
    }

    /**
     * @brief Creates an internal error.
     *
     * @param message Developer-facing diagnostic message.
     * @return SDK error.
     */
    [[nodiscard]] static Error internal(std::string message)
    {
      return Error{
          core_errors::ErrorCode::InternalError,
          std::move(message),
          core_errors::Severity::Critical};
    }

    /**
     * @brief Returns the machine-readable error code.
     */
    [[nodiscard]] core_errors::ErrorCode code() const noexcept
    {
      return code_;
    }

    /**
     * @brief Returns the error severity.
     */
    [[nodiscard]] core_errors::Severity severity() const noexcept
    {
      return severity_;
    }

    /**
     * @brief Returns the developer-facing message.
     */
    [[nodiscard]] const std::string &message() const noexcept
    {
      return message_;
    }

    /**
     * @brief Returns optional diagnostic context.
     */
    [[nodiscard]] const std::string &context() const noexcept
    {
      return context_;
    }

    /**
     * @brief Returns true when diagnostic context is available.
     */
    [[nodiscard]] bool has_context() const noexcept
    {
      return !context_.empty();
    }

    /**
     * @brief Returns true if this object represents an error.
     */
    [[nodiscard]] bool has_error() const noexcept
    {
      return code_ != core_errors::ErrorCode::None;
    }

    /**
     * @brief Returns true if this object represents success.
     */
    [[nodiscard]] bool ok() const noexcept
    {
      return code_ == core_errors::ErrorCode::None;
    }

    /**
     * @brief Returns the stable string representation of the error code.
     */
    [[nodiscard]] std::string_view code_string() const noexcept
    {
      return core_errors::to_string(code_);
    }

    /**
     * @brief Returns the stable string representation of the severity.
     */
    [[nodiscard]] std::string_view severity_string() const noexcept
    {
      return core_errors::to_string(severity_);
    }

    /**
     * @brief Converts the SDK error back to a core Softadastra error.
     */
    [[nodiscard]] core_errors::Error to_core() const
    {
      return core_errors::Error{
          code_,
          severity_,
          message_,
          core_errors::ErrorContext{context_}};
    }

  private:
    core_errors::ErrorCode code_{core_errors::ErrorCode::None};
    core_errors::Severity severity_{core_errors::Severity::Error};
    std::string message_{};
    std::string context_{};
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_ERROR_HPP

/*
 * Error.cpp
 */

#include <softadastra/sdk/Error.hpp>

#include <utility>

namespace softadastra::sdk
{
  namespace core_errors = softadastra::core::errors;

  Error::Error(
      Code code,
      std::string message,
      std::string context)
      : code_(code),
        message_(std::move(message)),
        context_(std::move(context))
  {
  }

  Error Error::none()
  {
    return Error{};
  }

  Error Error::make(
      Code code,
      std::string message,
      std::string context)
  {
    return Error{
        code,
        std::move(message),
        std::move(context)};
  }

  Error Error::invalid_argument(
      std::string message,
      std::string context)
  {
    return Error{
        Code::InvalidArgument,
        std::move(message),
        std::move(context)};
  }

  Error Error::invalid_state(
      std::string message,
      std::string context)
  {
    return Error{
        Code::InvalidState,
        std::move(message),
        std::move(context)};
  }

  Error Error::not_found(
      std::string message,
      std::string context)
  {
    return Error{
        Code::NotFound,
        std::move(message),
        std::move(context)};
  }

  Error Error::io_error(
      std::string message,
      std::string context)
  {
    return Error{
        Code::IoError,
        std::move(message),
        std::move(context)};
  }

  Error Error::sync_error(
      std::string message,
      std::string context)
  {
    return Error{
        Code::SyncError,
        std::move(message),
        std::move(context)};
  }

  Error Error::transport_error(
      std::string message,
      std::string context)
  {
    return Error{
        Code::TransportError,
        std::move(message),
        std::move(context)};
  }

  Error Error::discovery_error(
      std::string message,
      std::string context)
  {
    return Error{
        Code::DiscoveryError,
        std::move(message),
        std::move(context)};
  }

  Error Error::metadata_error(
      std::string message,
      std::string context)
  {
    return Error{
        Code::MetadataError,
        std::move(message),
        std::move(context)};
  }

  Error Error::internal_error(
      std::string message,
      std::string context)
  {
    return Error{
        Code::InternalError,
        std::move(message),
        std::move(context)};
  }

  Error Error::from_core(const core_errors::Error &error)
  {
    return Error{
        from_core_code(error.code()),
        error.message(),
        error.context().message()};
  }

  core_errors::Error Error::to_core() const
  {
    return core_errors::Error::make(
        to_core_code(code_),
        message_,
        core_errors::ErrorContext{context_});
  }

  Error::Code Error::code() const noexcept
  {
    return code_;
  }

  const std::string &Error::message() const noexcept
  {
    return message_;
  }

  const std::string &Error::context() const noexcept
  {
    return context_;
  }

  bool Error::has_context() const noexcept
  {
    return !context_.empty();
  }

  bool Error::ok() const noexcept
  {
    return code_ == Code::None;
  }

  bool Error::has_error() const noexcept
  {
    return code_ != Code::None;
  }

  void Error::clear() noexcept
  {
    code_ = Code::None;
    message_.clear();
    context_.clear();
  }

  std::string_view Error::to_string(Code code) noexcept
  {
    switch (code)
    {
    case Code::None:
      return "none";

    case Code::Unknown:
      return "unknown";

    case Code::InvalidArgument:
      return "invalid_argument";

    case Code::InvalidState:
      return "invalid_state";

    case Code::NotFound:
      return "not_found";

    case Code::AlreadyExists:
      return "already_exists";

    case Code::IoError:
      return "io_error";

    case Code::StoreError:
      return "store_error";

    case Code::SyncError:
      return "sync_error";

    case Code::TransportError:
      return "transport_error";

    case Code::DiscoveryError:
      return "discovery_error";

    case Code::MetadataError:
      return "metadata_error";

    case Code::InternalError:
      return "internal_error";

    default:
      return "unknown";
    }
  }

  std::string_view Error::code_string() const noexcept
  {
    return to_string(code_);
  }

  Error::Code Error::from_core_code(
      core_errors::ErrorCode code) noexcept
  {
    switch (code)
    {
    case core_errors::ErrorCode::None:
      return Code::None;

    case core_errors::ErrorCode::Unknown:
      return Code::Unknown;

    case core_errors::ErrorCode::InvalidArgument:
      return Code::InvalidArgument;

    case core_errors::ErrorCode::InvalidState:
      return Code::InvalidState;

    case core_errors::ErrorCode::NotFound:
      return Code::NotFound;

    case core_errors::ErrorCode::AlreadyExists:
      return Code::AlreadyExists;

    case core_errors::ErrorCode::FileNotFound:
    case core_errors::ErrorCode::FileAlreadyExists:
    case core_errors::ErrorCode::FileReadError:
    case core_errors::ErrorCode::FileWriteError:
    case core_errors::ErrorCode::PermissionDenied:
      return Code::IoError;

    case core_errors::ErrorCode::ConfigInvalid:
    case core_errors::ErrorCode::ConfigMissing:
      return Code::InvalidArgument;

    case core_errors::ErrorCode::HashError:
    case core_errors::ErrorCode::TimeError:
    case core_errors::ErrorCode::InternalError:
      return Code::InternalError;

    default:
      return Code::Unknown;
    }
  }

  core_errors::ErrorCode Error::to_core_code(Code code) noexcept
  {
    switch (code)
    {
    case Code::None:
      return core_errors::ErrorCode::None;

    case Code::Unknown:
      return core_errors::ErrorCode::Unknown;

    case Code::InvalidArgument:
      return core_errors::ErrorCode::InvalidArgument;

    case Code::InvalidState:
      return core_errors::ErrorCode::InvalidState;

    case Code::NotFound:
      return core_errors::ErrorCode::NotFound;

    case Code::AlreadyExists:
      return core_errors::ErrorCode::AlreadyExists;

    case Code::IoError:
      return core_errors::ErrorCode::FileReadError;

    case Code::StoreError:
    case Code::SyncError:
    case Code::TransportError:
    case Code::DiscoveryError:
    case Code::MetadataError:
    case Code::InternalError:
      return core_errors::ErrorCode::InternalError;

    default:
      return core_errors::ErrorCode::Unknown;
    }
  }

} // namespace softadastra::sdk

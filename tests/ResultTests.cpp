/*
 * ResultTests.cpp
 */

#include <cassert>
#include <string>

#include <softadastra/sdk/Result.hpp>
#include <softadastra/sdk/Error.hpp>

namespace
{
  using softadastra::sdk::Error;
  using softadastra::sdk::Result;

  void test_ok_result()
  {
    auto result = Result<int, Error>::ok(42);

    assert(result.is_ok());
    assert(!result.is_err());
    assert(result.value() == 42);
    assert(result.value_or(10) == 42);
  }

  void test_error_result()
  {
    auto result =
        Result<int, Error>::err(
            Error::invalid_argument("invalid value"));

    assert(!result.is_ok());
    assert(result.is_err());
    assert(result.error().has_error());
    assert(result.error().code() == Error::Code::InvalidArgument);
    assert(result.error().message() == "invalid value");
    assert(result.value_or(10) == 10);
  }

  void test_map_result()
  {
    auto result =
        Result<int, Error>::ok(21)
            .map([](int value)
                 { return value * 2; });

    assert(result.is_ok());
    assert(result.value() == 42);
  }

  void test_map_error_result()
  {
    auto result =
        Result<int, Error>::err(
            Error::not_found("missing key"))
            .map_error([](const Error &error)
                       { return Error::internal_error(
                             "wrapped: " + error.message()); });

    assert(result.is_err());
    assert(result.error().code() == Error::Code::InternalError);
    assert(result.error().message() == "wrapped: missing key");
  }

  void test_void_ok_result()
  {
    auto result = Result<void, Error>::ok();

    assert(result.is_ok());
    assert(!result.is_err());
  }

  void test_void_error_result()
  {
    auto result =
        Result<void, Error>::err(
            Error::invalid_state("client is closed"));

    assert(!result.is_ok());
    assert(result.is_err());
    assert(result.error().code() == Error::Code::InvalidState);
    assert(result.error().message() == "client is closed");
  }

  void test_string_result()
  {
    auto result =
        Result<std::string, Error>::ok(
            std::string{"softadastra"});

    assert(result.is_ok());
    assert(result.value() == "softadastra");
    assert(result.value_or("fallback") == "softadastra");
  }
}

int main()
{
  test_ok_result();
  test_error_result();
  test_map_result();
  test_map_error_result();
  test_void_ok_result();
  test_void_error_result();
  test_string_result();

  return 0;
}

/**
 *
 *  @file Value.hpp
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

#ifndef SOFTADASTRA_SDK_VALUE_HPP
#define SOFTADASTRA_SDK_VALUE_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <softadastra/store/types/Value.hpp>

namespace softadastra::sdk
{
  namespace store_types = softadastra::store::types;

  /**
   * @brief Public SDK value type.
   *
   * Value stores binary-safe data exposed by the SDK client.
   *
   * It is intentionally a small wrapper over bytes:
   * - strings are stored as raw bytes
   * - binary payloads are supported
   * - empty values are allowed
   *
   * The SDK does not interpret the content. Higher-level applications decide
   * whether the bytes represent text, JSON, files, metadata, or another format.
   */
  class Value
  {
  public:
    /**
     * @brief Internal byte container type.
     */
    using Container = std::vector<std::uint8_t>;

    /**
     * @brief Creates an empty value.
     */
    Value() = default;

    /**
     * @brief Creates a value from bytes.
     *
     * @param data Binary payload.
     */
    explicit Value(Container data)
        : data_(std::move(data))
    {
    }

    /**
     * @brief Creates a value from a byte span.
     *
     * @param data Binary payload.
     */
    explicit Value(std::span<const std::uint8_t> data)
        : data_(data.begin(), data.end())
    {
    }

    /**
     * @brief Creates a value from a string view.
     *
     * The string is stored as raw bytes.
     *
     * @param data Text or binary-compatible payload.
     */
    explicit Value(std::string_view data)
        : data_(data.begin(), data.end())
    {
    }

    /**
     * @brief Creates a value from a C string.
     *
     * Null pointers are converted to an empty value.
     *
     * @param data Text payload.
     */
    explicit Value(const char *data)
        : Value(data == nullptr ? std::string_view{} : std::string_view{data})
    {
    }

    /**
     * @brief Creates a value from bytes.
     *
     * @param data Binary payload.
     * @return SDK value.
     */
    [[nodiscard]] static Value from_bytes(Container data)
    {
      return Value{std::move(data)};
    }

    /**
     * @brief Creates a value from a byte span.
     *
     * @param data Binary payload.
     * @return SDK value.
     */
    [[nodiscard]] static Value from_span(std::span<const std::uint8_t> data)
    {
      return Value{data};
    }

    /**
     * @brief Creates a value from text.
     *
     * @param data Text payload.
     * @return SDK value.
     */
    [[nodiscard]] static Value from_string(std::string_view data)
    {
      return Value{data};
    }

    /**
     * @brief Creates a value from an internal Softadastra Store value.
     *
     * @param value Internal store value.
     * @return SDK value.
     */
    [[nodiscard]] static Value from_store(const store_types::Value &value)
    {
      return Value{value.bytes()};
    }

    /**
     * @brief Returns the stored bytes.
     *
     * @return Read-only byte container.
     */
    [[nodiscard]] const Container &bytes() const noexcept
    {
      return data_;
    }

    /**
     * @brief Returns the stored bytes.
     *
     * Alias for bytes().
     *
     * @return Read-only byte container.
     */
    [[nodiscard]] const Container &data() const noexcept
    {
      return data_;
    }

    /**
     * @brief Returns the stored bytes as a mutable container.
     *
     * @return Mutable byte container.
     */
    [[nodiscard]] Container &data() noexcept
    {
      return data_;
    }

    /**
     * @brief Returns a byte span over the stored data.
     *
     * @return Read-only byte span.
     */
    [[nodiscard]] std::span<const std::uint8_t> span() const noexcept
    {
      return std::span<const std::uint8_t>{data_.data(), data_.size()};
    }

    /**
     * @brief Returns true if the value has no bytes.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return data_.empty();
    }

    /**
     * @brief Returns the number of stored bytes.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
      return data_.size();
    }

    /**
     * @brief Clears the value.
     */
    void clear() noexcept
    {
      data_.clear();
    }

    /**
     * @brief Converts the value bytes to a string.
     *
     * This does not validate UTF-8.
     *
     * @return String built from the stored bytes.
     */
    [[nodiscard]] std::string to_string() const
    {
      return std::string{data_.begin(), data_.end()};
    }

    /**
     * @brief Converts this SDK value to an internal Softadastra Store value.
     */
    [[nodiscard]] store_types::Value to_store() const
    {
      return store_types::Value{data_};
    }

    /**
     * @brief Compares two values for equality.
     */
    [[nodiscard]] friend bool operator==(
        const Value &a,
        const Value &b)
    {
      return a.data_ == b.data_;
    }

    /**
     * @brief Compares two values for inequality.
     */
    [[nodiscard]] friend bool operator!=(
        const Value &a,
        const Value &b)
    {
      return !(a == b);
    }

  private:
    Container data_{};
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_VALUE_HPP

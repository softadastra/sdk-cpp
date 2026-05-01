/**
 *
 *  @file Key.hpp
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

#ifndef SOFTADASTRA_SDK_KEY_HPP
#define SOFTADASTRA_SDK_KEY_HPP

#include <string>
#include <string_view>
#include <utility>

#include <softadastra/store/types/Key.hpp>

namespace softadastra::sdk
{
  namespace store_types = softadastra::store::types;

  /**
   * @brief Public SDK key type.
   *
   * Key identifies a value stored through the SDK client.
   *
   * It intentionally stays string-based because Softadastra Store already uses
   * stable string keys internally. The SDK wrapper keeps the public API clean
   * while allowing conversion to the internal store key when needed.
   *
   * Rules:
   * - A valid key must not be empty.
   * - Keys are stable and safe to persist.
   * - Keys are compared lexicographically.
   */
  class Key
  {
  public:
    /**
     * @brief Creates an empty key.
     */
    Key() = default;

    /**
     * @brief Creates a key from a string.
     *
     * @param value Key value.
     */
    explicit Key(std::string value)
        : value_(std::move(value))
    {
    }

    /**
     * @brief Creates a key from a string view.
     *
     * @param value Key value.
     */
    explicit Key(std::string_view value)
        : value_(value)
    {
    }

    /**
     * @brief Creates a key from a C string.
     *
     * Null pointers are converted to an empty key.
     *
     * @param value Key value.
     */
    explicit Key(const char *value)
        : value_(value == nullptr ? "" : value)
    {
    }

    /**
     * @brief Creates a key from a string.
     *
     * @param value Key value.
     * @return SDK key.
     */
    [[nodiscard]] static Key from(std::string value)
    {
      return Key{std::move(value)};
    }

    /**
     * @brief Creates a key from a string view.
     *
     * @param value Key value.
     * @return SDK key.
     */
    [[nodiscard]] static Key from_view(std::string_view value)
    {
      return Key{value};
    }

    /**
     * @brief Creates a key from an internal Softadastra Store key.
     *
     * @param key Internal store key.
     * @return SDK key.
     */
    [[nodiscard]] static Key from_store(const store_types::Key &key)
    {
      return Key{key.str()};
    }

    /**
     * @brief Returns the key string.
     *
     * @return Key value.
     */
    [[nodiscard]] const std::string &str() const noexcept
    {
      return value_;
    }

    /**
     * @brief Returns the key string.
     *
     * Alias for str().
     *
     * @return Key value.
     */
    [[nodiscard]] const std::string &value() const noexcept
    {
      return value_;
    }

    /**
     * @brief Returns true if the key is empty.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return value_.empty();
    }

    /**
     * @brief Returns true if the key can be used by the SDK.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !value_.empty();
    }

    /**
     * @brief Backward-compatible valid alias.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Clears the key.
     */
    void clear() noexcept
    {
      value_.clear();
    }

    /**
     * @brief Converts this SDK key to an internal Softadastra Store key.
     */
    [[nodiscard]] store_types::Key to_store() const
    {
      return store_types::Key{value_};
    }

    /**
     * @brief Compares two keys for equality.
     */
    [[nodiscard]] friend bool operator==(
        const Key &a,
        const Key &b) noexcept
    {
      return a.value_ == b.value_;
    }

    /**
     * @brief Compares two keys for inequality.
     */
    [[nodiscard]] friend bool operator!=(
        const Key &a,
        const Key &b) noexcept
    {
      return !(a == b);
    }

    /**
     * @brief Orders keys lexicographically.
     */
    [[nodiscard]] friend bool operator<(
        const Key &a,
        const Key &b) noexcept
    {
      return a.value_ < b.value_;
    }

  private:
    std::string value_{};
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_KEY_HPP

/**
 *
 *  @file ClientOptions.hpp
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

#ifndef SOFTADASTRA_SDK_CLIENT_OPTIONS_HPP
#define SOFTADASTRA_SDK_CLIENT_OPTIONS_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/metadata/MetadataOptions.hpp>

namespace softadastra::sdk
{
  /**
   * @brief Public configuration object for the SDK client.
   *
   * ClientOptions groups the user-facing configuration needed to initialize
   * a Softadastra SDK client.
   *
   * It controls:
   * - local node identity
   * - local WAL-backed store
   * - sync behavior
   * - optional transport configuration
   * - optional discovery configuration
   * - optional metadata configuration
   *
   * The SDK keeps this object small and explicit. Internal module-specific
   * config objects are generated from it through conversion helpers.
   */
  struct ClientOptions
  {
    /**
     * @brief Local logical node identifier.
     *
     * This value should be unique per device or peer.
     */
    std::string node_id{"node-1"};

    /**
     * @brief Path to the SDK local store WAL.
     *
     * The store uses this WAL to persist local writes before they are applied.
     */
    std::string store_wal_path{"data/sdk-store.wal"};

    /**
     * @brief Enable WAL persistence for the local store.
     *
     * When false, the SDK client uses an in-memory store.
     */
    bool enable_store_wal{true};

    /**
     * @brief Flush the WAL after every accepted local write.
     *
     * This is the safest production default.
     */
    bool auto_flush{true};

    /**
     * @brief Initial in-memory store capacity hint.
     */
    std::size_t initial_store_capacity{1024};

    /**
     * @brief Maximum number of sync operations returned in one batch.
     */
    std::size_t sync_batch_size{64};

    /**
     * @brief Maximum number of sync retries before giving up.
     */
    std::uint32_t max_sync_retries{5};

    /**
     * @brief Enable acknowledgement tracking for outbound sync operations.
     */
    bool require_ack{true};

    /**
     * @brief Automatically queue local operations after submission.
     */
    bool auto_queue{true};

    /**
     * @brief Enable transport support in the SDK client.
     *
     * Transport is optional because local-only apps should not be forced to
     * start networking.
     */
    bool enable_transport{false};

    /**
     * @brief Local transport bind host.
     */
    std::string transport_host{"0.0.0.0"};

    /**
     * @brief Local transport bind port.
     */
    std::uint16_t transport_port{0};

    /**
     * @brief Enable peer discovery support in the SDK client.
     */
    bool enable_discovery{false};

    /**
     * @brief Discovery bind host.
     */
    std::string discovery_host{"0.0.0.0"};

    /**
     * @brief Discovery bind port.
     */
    std::uint16_t discovery_port{0};

    /**
     * @brief Human-readable node display name.
     */
    std::string display_name{};

    /**
     * @brief Product or runtime version exposed by metadata.
     */
    std::string version{"0.1.0"};

    /**
     * @brief Creates default client options.
     */
    ClientOptions() = default;

    /**
     * @brief Creates client options for a local node.
     *
     * @param local_node_id Local node id.
     */
    explicit ClientOptions(std::string local_node_id)
        : node_id(std::move(local_node_id))
    {
      if (display_name.empty())
      {
        display_name = node_id;
      }
    }

    /**
     * @brief Creates durable production-oriented options.
     *
     * @param local_node_id Local node id.
     * @param wal_path Store WAL path.
     * @return ClientOptions.
     */
    [[nodiscard]] static ClientOptions durable(
        std::string local_node_id,
        std::string wal_path = "data/sdk-store.wal")
    {
      ClientOptions options{std::move(local_node_id)};
      options.store_wal_path = std::move(wal_path);
      options.enable_store_wal = true;
      options.auto_flush = true;
      options.initial_store_capacity = 1024;
      options.sync_batch_size = 64;
      options.max_sync_retries = 5;
      options.require_ack = true;
      options.auto_queue = true;
      return options;
    }

    /**
     * @brief Creates fast local options for tests or demos.
     *
     * @param local_node_id Local node id.
     * @param wal_path Store WAL path.
     * @return ClientOptions.
     */
    [[nodiscard]] static ClientOptions fast(
        std::string local_node_id,
        std::string wal_path = "data/sdk-store.wal")
    {
      ClientOptions options{std::move(local_node_id)};
      options.store_wal_path = std::move(wal_path);
      options.enable_store_wal = true;
      options.auto_flush = false;
      options.initial_store_capacity = 1024;
      options.sync_batch_size = 128;
      options.max_sync_retries = 2;
      options.require_ack = false;
      options.auto_queue = true;
      return options;
    }

    /**
     * @brief Creates memory-only options.
     *
     * @param local_node_id Local node id.
     * @return ClientOptions.
     */
    [[nodiscard]] static ClientOptions memory_only(
        std::string local_node_id)
    {
      ClientOptions options{std::move(local_node_id)};
      options.enable_store_wal = false;
      options.auto_flush = false;
      options.initial_store_capacity = 1024;
      options.require_ack = false;
      return options;
    }

    /**
     * @brief Enables transport and returns this options object.
     *
     * @param host Bind host.
     * @param port Bind port.
     * @return Modified options.
     */
    [[nodiscard]] ClientOptions with_transport(
        std::string host,
        std::uint16_t port) const
    {
      ClientOptions options = *this;
      options.enable_transport = true;
      options.transport_host = std::move(host);
      options.transport_port = port;
      return options;
    }

    /**
     * @brief Enables local transport and returns this options object.
     *
     * @param port Bind port.
     * @return Modified options.
     */
    [[nodiscard]] ClientOptions with_local_transport(
        std::uint16_t port) const
    {
      return with_transport("127.0.0.1", port);
    }

    /**
     * @brief Enables discovery and returns this options object.
     *
     * @param host Bind host.
     * @param port Bind port.
     * @return Modified options.
     */
    [[nodiscard]] ClientOptions with_discovery(
        std::string host,
        std::uint16_t port) const
    {
      ClientOptions options = *this;
      options.enable_discovery = true;
      options.discovery_host = std::move(host);
      options.discovery_port = port;
      return options;
    }

    /**
     * @brief Enables local discovery and returns this options object.
     *
     * @param port Bind port.
     * @return Modified options.
     */
    [[nodiscard]] ClientOptions with_local_discovery(
        std::uint16_t port) const
    {
      return with_discovery("127.0.0.1", port);
    }

    /**
     * @brief Sets display metadata and returns this options object.
     *
     * @param name Human-readable node display name.
     * @param runtime_version Runtime or product version.
     * @return Modified options.
     */
    [[nodiscard]] ClientOptions with_metadata(
        std::string name,
        std::string runtime_version = "0.1.0") const
    {
      ClientOptions options = *this;
      options.display_name = std::move(name);
      options.version = std::move(runtime_version);
      return options;
    }

    /**
     * @brief Converts SDK options to StoreConfig.
     */
    [[nodiscard]] softadastra::store::core::StoreConfig
    to_store_config() const
    {
      if (!enable_store_wal)
      {
        auto config = softadastra::store::core::StoreConfig::memory_only();
        config.initial_capacity = initial_store_capacity;
        return config;
      }

      auto config =
          softadastra::store::core::StoreConfig::durable(store_wal_path);

      config.enable_wal = enable_store_wal;
      config.auto_flush = auto_flush;
      config.initial_capacity = initial_store_capacity;

      return config;
    }

    /**
     * @brief Converts SDK options to SyncConfig.
     */
    [[nodiscard]] softadastra::sync::core::SyncConfig
    to_sync_config() const
    {
      auto config =
          softadastra::sync::core::SyncConfig::durable(node_id);

      config.batch_size = sync_batch_size;
      config.max_retries = max_sync_retries;
      config.require_ack = require_ack;
      config.auto_queue = auto_queue;

      return config;
    }

    /**
     * @brief Converts SDK options to TransportConfig.
     */
    [[nodiscard]] softadastra::transport::core::TransportConfig
    to_transport_config() const
    {
      return softadastra::transport::core::TransportConfig{
          transport_host,
          transport_port};
    }

    /**
     * @brief Converts SDK options to DiscoveryOptions.
     */
    [[nodiscard]] softadastra::discovery::DiscoveryOptions
    to_discovery_options() const
    {
      softadastra::discovery::DiscoveryOptions options;
      options.node_id = node_id;
      options.bind_host = discovery_host;
      options.bind_port = discovery_port;
      return options;
    }

    /**
     * @brief Converts SDK options to MetadataOptions.
     */
    [[nodiscard]] softadastra::metadata::MetadataOptions
    to_metadata_options() const
    {
      softadastra::metadata::MetadataOptions options;
      options.node_id = node_id;
      options.display_name = display_name.empty() ? node_id : display_name;
      options.version = version;
      return options;
    }

    /**
     * @brief Returns true if the options are usable.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      if (node_id.empty())
      {
        return false;
      }

      if (version.empty())
      {
        return false;
      }

      if (initial_store_capacity == 0)
      {
        return false;
      }

      if (enable_store_wal && store_wal_path.empty())
      {
        return false;
      }

      if (sync_batch_size == 0)
      {
        return false;
      }

      if (enable_transport &&
          (transport_host.empty() || transport_port == 0))
      {
        return false;
      }

      if (enable_discovery &&
          (discovery_host.empty() || discovery_port == 0))
      {
        return false;
      }

      return true;
    }

    /**
     * @brief Backward-compatible valid alias.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_CLIENT_OPTIONS_HPP

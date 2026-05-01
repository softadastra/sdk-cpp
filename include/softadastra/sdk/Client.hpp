/**
 *
 *  @file Client.hpp
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

#ifndef SOFTADASTRA_SDK_CLIENT_HPP
#define SOFTADASTRA_SDK_CLIENT_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <softadastra/sdk/ClientOptions.hpp>
#include <softadastra/sdk/Error.hpp>
#include <softadastra/sdk/Key.hpp>
#include <softadastra/sdk/NodeInfo.hpp>
#include <softadastra/sdk/Peer.hpp>
#include <softadastra/sdk/Result.hpp>
#include <softadastra/sdk/SyncResult.hpp>
#include <softadastra/sdk/TickResult.hpp>
#include <softadastra/sdk/Value.hpp>

#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/sync/scheduler/SyncScheduler.hpp>

#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/discovery/DiscoveryService.hpp>
#include <softadastra/metadata/MetadataOptions.hpp>
#include <softadastra/metadata/MetadataService.hpp>

namespace softadastra::sdk
{
  namespace store_engine = softadastra::store::engine;
  namespace store_core = softadastra::store::core;
  namespace store_types = softadastra::store::types;

  namespace sync_engine = softadastra::sync::engine;
  namespace sync_core = softadastra::sync::core;
  namespace sync_scheduler = softadastra::sync::scheduler;

  namespace transport_core = softadastra::transport::core;
  namespace transport_engine = softadastra::transport::engine;

  namespace discovery_api = softadastra::discovery;
  namespace metadata_api = softadastra::metadata;

  /**
   * @brief High-level Softadastra C++ SDK client.
   *
   * Client is the main developer-facing API of sdk-cpp.
   *
   * It hides the internal Softadastra modules behind a small and stable API:
   * - local durable key-value writes
   * - WAL-backed persistence through StoreEngine
   * - sync queue management through SyncEngine
   * - manual sync ticks through SyncScheduler
   * - optional transport
   * - optional discovery
   * - optional node metadata
   *
   * The SDK client does not force networking. A local-only client can use the
   * store and sync pipeline without starting transport or discovery.
   */
  class Client
  {
  public:
    /**
     * @brief Result returned by operations without a value.
     */
    using VoidResult = Result<void, Error>;

    /**
     * @brief Result returned by get().
     */
    using ValueResult = Result<Value, Error>;

    /**
     * @brief Result returned by sync inspection methods.
     */
    using SyncStateResult = Result<SyncResult, Error>;

    /**
     * @brief Result returned by tick().
     */
    using TickStateResult = Result<TickResult, Error>;

    /**
     * @brief Result returned by node_info().
     */
    using NodeInfoResult = Result<NodeInfo, Error>;

    /**
     * @brief Result returned by peers().
     */
    using PeersResult = Result<std::vector<Peer>, Error>;

    /**
     * @brief Creates a client with default options.
     */
    Client();

    /**
     * @brief Creates a client with explicit options.
     *
     * @param options SDK client options.
     */
    explicit Client(ClientOptions options);

    /**
     * @brief Stops owned services before destruction.
     */
    ~Client();

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(Client &&) noexcept;
    Client &operator=(Client &&) noexcept;

    /**
     * @brief Opens and initializes the SDK client.
     *
     * This initializes the local store, sync engine, scheduler, and optional
     * service objects according to ClientOptions.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult open();

    /**
     * @brief Closes the SDK client and stops optional services.
     */
    void close() noexcept;

    /**
     * @brief Returns true if the client is open.
     */
    [[nodiscard]] bool is_open() const noexcept;

    /**
     * @brief Backward-compatible open alias.
     */
    [[nodiscard]] bool opened() const noexcept;

    /**
     * @brief Stores a value for a key.
     *
     * The write is applied to the local store and submitted to the sync layer
     * when sync is available.
     *
     * @param key SDK key.
     * @param value SDK value.
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult put(
        const Key &key,
        const Value &value);

    /**
     * @brief Stores a string value for a key.
     *
     * Convenience overload for text payloads.
     *
     * @param key SDK key.
     * @param value Text value.
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult put(
        std::string key,
        std::string value);

    /**
     * @brief Reads a value from the local store.
     *
     * @param key SDK key.
     * @return Value on success, Error on failure.
     */
    [[nodiscard]] ValueResult get(const Key &key) const;

    /**
     * @brief Reads a value from the local store by string key.
     *
     * @param key Key string.
     * @return Value on success, Error on failure.
     */
    [[nodiscard]] ValueResult get(const std::string &key) const;

    /**
     * @brief Removes a value from the local store.
     *
     * The deletion is submitted to the sync layer when sync is available.
     *
     * @param key SDK key.
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult remove(const Key &key);

    /**
     * @brief Removes a value from the local store by string key.
     *
     * @param key Key string.
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult remove(const std::string &key);

    /**
     * @brief Returns true if the local store contains a key.
     *
     * @param key SDK key.
     * @return true if present.
     */
    [[nodiscard]] bool contains(const Key &key) const;

    /**
     * @brief Returns true if the local store contains a string key.
     *
     * @param key Key string.
     * @return true if present.
     */
    [[nodiscard]] bool contains(const std::string &key) const;

    /**
     * @brief Returns the number of entries in the local store.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Returns true if the local store is empty.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Advances the sync pipeline once.
     *
     * This is deterministic and manual. It retries expired operations,
     * collects a batch ready for transport, and optionally prunes completed
     * entries.
     *
     * @param prune_completed Remove completed entries after the tick.
     * @return SDK tick result.
     */
    [[nodiscard]] TickStateResult tick(
        bool prune_completed = false);

    /**
     * @brief Returns a compact synchronization state.
     *
     * @return SDK sync result.
     */
    [[nodiscard]] SyncStateResult sync_state() const;

    /**
     * @brief Retries expired sync operations.
     *
     * @return Number of operations requeued.
     */
    [[nodiscard]] Result<std::size_t, Error> retry_expired();

    /**
     * @brief Prunes completed sync entries.
     *
     * @return Number of entries removed.
     */
    [[nodiscard]] Result<std::size_t, Error> prune_completed();

    /**
     * @brief Prunes failed sync entries.
     *
     * @return Number of entries removed.
     */
    [[nodiscard]] Result<std::size_t, Error> prune_failed();

    /**
     * @brief Starts optional transport support.
     *
     * Requires ClientOptions::enable_transport to be true.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult start_transport();

    /**
     * @brief Stops transport if it is available.
     */
    void stop_transport() noexcept;

    /**
     * @brief Returns true if transport is enabled and running.
     */
    [[nodiscard]] bool transport_running() const noexcept;

    /**
     * @brief Connects to a peer through transport.
     *
     * @param peer Remote peer.
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult connect(const Peer &peer);

    /**
     * @brief Disconnects from a peer through transport.
     *
     * @param peer Remote peer.
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult disconnect(const Peer &peer);

    /**
     * @brief Starts optional discovery support.
     *
     * Requires ClientOptions::enable_discovery to be true.
     *
     * @return Result<void, Error>.
     */
    [[nodiscard]] VoidResult start_discovery();

    /**
     * @brief Stops discovery if it is available.
     */
    void stop_discovery() noexcept;

    /**
     * @brief Returns true if discovery is enabled and running.
     */
    [[nodiscard]] bool discovery_running() const noexcept;

    /**
     * @brief Returns discovered peers.
     *
     * @return List of SDK peers.
     */
    [[nodiscard]] PeersResult peers() const;

    /**
     * @brief Returns local node metadata as SDK NodeInfo.
     *
     * @return SDK node info.
     */
    [[nodiscard]] NodeInfoResult node_info() const;

    /**
     * @brief Refreshes local node metadata and returns it as SDK NodeInfo.
     *
     * @return SDK node info.
     */
    [[nodiscard]] NodeInfoResult refresh_node_info();

    /**
     * @brief Returns the SDK options used by this client.
     */
    [[nodiscard]] const ClientOptions &options() const noexcept;

    /**
     * @brief Returns the internal store engine.
     *
     * This is exposed for advanced integrations. Most applications should use
     * the SDK-level put/get/remove methods instead.
     */
    [[nodiscard]] store_engine::StoreEngine &store();

    /**
     * @brief Returns the internal store engine.
     */
    [[nodiscard]] const store_engine::StoreEngine &store() const;

    /**
     * @brief Returns the internal sync engine.
     *
     * This is exposed for advanced integrations.
     */
    [[nodiscard]] sync_engine::SyncEngine &sync();

    /**
     * @brief Returns the internal sync engine.
     */
    [[nodiscard]] const sync_engine::SyncEngine &sync() const;

  private:
    /**
     * @brief Ensures the client is open.
     */
    [[nodiscard]] VoidResult require_open() const;

    /**
     * @brief Ensures transport is available.
     */
    [[nodiscard]] VoidResult require_transport() const;

    /**
     * @brief Ensures discovery is available.
     */
    [[nodiscard]] VoidResult require_discovery() const;

    /**
     * @brief Ensures metadata is available.
     */
    [[nodiscard]] VoidResult require_metadata() const;

    /**
     * @brief Builds internal runtime objects from options.
     */
    void build_runtime();

  private:
    ClientOptions options_{};

    std::unique_ptr<store_engine::StoreEngine> store_{};
    std::unique_ptr<sync_engine::SyncEngine> sync_{};
    std::unique_ptr<sync_scheduler::SyncScheduler> scheduler_{};

    std::unique_ptr<transport_core::TransportConfig> transport_config_{};
    std::unique_ptr<transport_core::TransportContext> transport_context_{};
    std::unique_ptr<transport_engine::TransportEngine> transport_{};

    std::unique_ptr<discovery_api::DiscoveryService> discovery_{};
    std::unique_ptr<metadata_api::MetadataService> metadata_{};

    bool open_{false};
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_CLIENT_HPP

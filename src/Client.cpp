/*
 * Client.cpp
 */

#include <softadastra/sdk/Client.hpp>

#include <utility>

#include <softadastra/store/core/Operation.hpp>
#include <softadastra/store/types/OperationType.hpp>

namespace softadastra::sdk
{
  namespace store_core = softadastra::store::core;
  namespace store_types = softadastra::store::types;

  Client::Client()
      : Client(ClientOptions{})
  {
  }

  Client::Client(ClientOptions options)
      : options_(std::move(options))
  {
  }

  Client::~Client()
  {
    close();
  }

  Client::Client(Client &&other) noexcept
      : options_(std::move(other.options_)),
        store_(std::move(other.store_)),
        sync_(std::move(other.sync_)),
        scheduler_(std::move(other.scheduler_)),
        transport_config_(std::move(other.transport_config_)),
        transport_context_(std::move(other.transport_context_)),
        transport_(std::move(other.transport_)),
        discovery_(std::move(other.discovery_)),
        metadata_(std::move(other.metadata_)),
        open_(other.open_)
  {
    other.open_ = false;
  }

  Client &Client::operator=(Client &&other) noexcept
  {
    if (this != &other)
    {
      close();

      options_ = std::move(other.options_);
      store_ = std::move(other.store_);
      sync_ = std::move(other.sync_);
      scheduler_ = std::move(other.scheduler_);
      transport_config_ = std::move(other.transport_config_);
      transport_context_ = std::move(other.transport_context_);
      transport_ = std::move(other.transport_);
      discovery_ = std::move(other.discovery_);
      metadata_ = std::move(other.metadata_);
      open_ = other.open_;

      other.open_ = false;
    }

    return *this;
  }

  Client::VoidResult Client::open()
  {
    if (open_)
    {
      return VoidResult::ok();
    }

    if (!options_.is_valid())
    {
      return VoidResult::err(
          Error::invalid_argument(
              "invalid SDK client options"));
    }

    build_runtime();

    if (!store_ || !sync_ || !scheduler_)
    {
      return VoidResult::err(
          Error::internal_error(
              "failed to build SDK runtime"));
    }

    open_ = true;

    return VoidResult::ok();
  }

  void Client::close() noexcept
  {
    stop_discovery();
    stop_transport();

    metadata_.reset();
    discovery_.reset();

    transport_.reset();
    transport_context_.reset();
    transport_config_.reset();

    scheduler_.reset();
    sync_.reset();
    store_.reset();

    open_ = false;
  }

  bool Client::is_open() const noexcept
  {
    return open_;
  }

  bool Client::opened() const noexcept
  {
    return is_open();
  }

  Client::VoidResult Client::put(
      const Key &key,
      const Value &value)
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    if (!key.is_valid())
    {
      return VoidResult::err(
          Error::invalid_argument(
              "key cannot be empty"));
    }

    store_core::Operation operation =
        store_core::Operation::put(
            key.to_store(),
            value.to_store());

    const auto applied = store_->apply(operation);

    if (applied.is_err())
    {
      return VoidResult::err(
          Error::from_core(applied.error()));
    }

    const auto submitted = sync_->submit_local(operation);

    if (submitted.is_err())
    {
      return VoidResult::err(
          Error::from_core(submitted.error()));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::put(
      std::string key,
      std::string value)
  {
    return put(
        Key{std::move(key)},
        Value::from_string(value));
  }

  Client::ValueResult Client::get(const Key &key) const
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return ValueResult::err(open_result.error());
    }

    if (!key.is_valid())
    {
      return ValueResult::err(
          Error::invalid_argument(
              "key cannot be empty"));
    }

    const auto *entry = store_->get(key.to_store());

    if (entry == nullptr)
    {
      return ValueResult::err(
          Error::not_found(
              "key not found",
              key.str()));
    }

    return ValueResult::ok(
        Value::from_store(entry->value));
  }

  Client::ValueResult Client::get(const std::string &key) const
  {
    return get(Key{key});
  }

  Client::VoidResult Client::remove(const Key &key)
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    if (!key.is_valid())
    {
      return VoidResult::err(
          Error::invalid_argument(
              "key cannot be empty"));
    }

    store_core::Operation operation =
        store_core::Operation::remove(key.to_store());

    const auto applied = store_->apply(operation);

    if (applied.is_err())
    {
      return VoidResult::err(
          Error::from_core(applied.error()));
    }

    const auto submitted = sync_->submit_local(operation);

    if (submitted.is_err())
    {
      return VoidResult::err(
          Error::from_core(submitted.error()));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::remove(const std::string &key)
  {
    return remove(Key{key});
  }

  bool Client::contains(const Key &key) const
  {
    if (!open_ || !store_ || !key.is_valid())
    {
      return false;
    }

    return store_->contains(key.to_store());
  }

  bool Client::contains(const std::string &key) const
  {
    return contains(Key{key});
  }

  std::size_t Client::size() const noexcept
  {
    if (!open_ || !store_)
    {
      return 0;
    }

    return store_->size();
  }

  bool Client::empty() const noexcept
  {
    return size() == 0;
  }

  Client::TickStateResult Client::tick(bool prune_completed)
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return TickStateResult::err(open_result.error());
    }

    if (!scheduler_)
    {
      return TickStateResult::err(
          Error::invalid_state(
              "sync scheduler is not available"));
    }

    const auto result = scheduler_->tick(prune_completed);

    return TickStateResult::ok(
        TickResult::from_scheduler(result));
  }

  Client::SyncStateResult Client::sync_state() const
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return SyncStateResult::err(open_result.error());
    }

    return SyncStateResult::ok(
        SyncResult::from_state(sync_->state()));
  }

  Result<std::size_t, Error> Client::retry_expired()
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return Result<std::size_t, Error>::err(open_result.error());
    }

    return Result<std::size_t, Error>::ok(
        scheduler_->retry_only());
  }

  Result<std::size_t, Error> Client::prune_completed()
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return Result<std::size_t, Error>::err(open_result.error());
    }

    return Result<std::size_t, Error>::ok(
        scheduler_->prune_completed());
  }

  Result<std::size_t, Error> Client::prune_failed()
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return Result<std::size_t, Error>::err(open_result.error());
    }

    return Result<std::size_t, Error>::ok(
        scheduler_->prune_failed());
  }

  Client::VoidResult Client::start_transport()
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    auto transport_result = require_transport();

    if (transport_result.is_err())
    {
      return transport_result;
    }

    if (transport_->is_running())
    {
      return VoidResult::ok();
    }

    if (!transport_->start())
    {
      return VoidResult::err(
          Error::transport_error(
              "failed to start transport"));
    }

    return VoidResult::ok();
  }

  void Client::stop_transport() noexcept
  {
    if (transport_)
    {
      transport_->stop();
    }
  }

  bool Client::transport_running() const noexcept
  {
    return transport_ && transport_->is_running();
  }

  Client::VoidResult Client::connect(const Peer &peer)
  {
    auto transport_result = require_transport();

    if (transport_result.is_err())
    {
      return transport_result;
    }

    if (!peer.is_valid())
    {
      return VoidResult::err(
          Error::invalid_argument(
              "invalid peer"));
    }

    if (!transport_->connect(peer.to_transport()))
    {
      return VoidResult::err(
          Error::transport_error(
              "failed to connect to peer",
              peer.node_id));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::disconnect(const Peer &peer)
  {
    auto transport_result = require_transport();

    if (transport_result.is_err())
    {
      return transport_result;
    }

    if (!peer.is_valid())
    {
      return VoidResult::err(
          Error::invalid_argument(
              "invalid peer"));
    }

    if (!transport_->disconnect(peer.to_transport()))
    {
      return VoidResult::err(
          Error::transport_error(
              "failed to disconnect from peer",
              peer.node_id));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::start_discovery()
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    auto discovery_result = require_discovery();

    if (discovery_result.is_err())
    {
      return discovery_result;
    }

    if (discovery_->is_running())
    {
      return VoidResult::ok();
    }

    if (!discovery_->start())
    {
      return VoidResult::err(
          Error::discovery_error(
              "failed to start discovery"));
    }

    return VoidResult::ok();
  }

  void Client::stop_discovery() noexcept
  {
    if (discovery_)
    {
      discovery_->stop();
    }
  }

  bool Client::discovery_running() const noexcept
  {
    return discovery_ && discovery_->is_running();
  }

  Client::PeersResult Client::peers() const
  {
    auto discovery_result = require_discovery();

    if (discovery_result.is_err())
    {
      return PeersResult::err(discovery_result.error());
    }

    std::vector<Peer> result;

    for (const auto &peer : discovery_->peers())
    {
      result.push_back(Peer::from_discovery(peer));
    }

    return PeersResult::ok(std::move(result));
  }

  Client::NodeInfoResult Client::node_info() const
  {
    auto metadata_result = require_metadata();

    if (metadata_result.is_err())
    {
      return NodeInfoResult::err(metadata_result.error());
    }

    const auto metadata = metadata_->local_metadata();

    if (!metadata.has_value())
    {
      return NodeInfoResult::err(
          Error::metadata_error(
              "local node metadata is not available"));
    }

    return NodeInfoResult::ok(
        NodeInfo::from_metadata(*metadata));
  }

  Client::NodeInfoResult Client::refresh_node_info()
  {
    auto metadata_result = require_metadata();

    if (metadata_result.is_err())
    {
      return NodeInfoResult::err(metadata_result.error());
    }

    const auto metadata = metadata_->refresh_local_metadata();

    if (!metadata.has_value())
    {
      return NodeInfoResult::err(
          Error::metadata_error(
              "failed to refresh local node metadata"));
    }

    return NodeInfoResult::ok(
        NodeInfo::from_metadata(*metadata));
  }

  const ClientOptions &Client::options() const noexcept
  {
    return options_;
  }

  store_engine::StoreEngine &Client::store()
  {
    return *store_;
  }

  const store_engine::StoreEngine &Client::store() const
  {
    return *store_;
  }

  sync_engine::SyncEngine &Client::sync()
  {
    return *sync_;
  }

  const sync_engine::SyncEngine &Client::sync() const
  {
    return *sync_;
  }

  Client::VoidResult Client::require_open() const
  {
    if (!open_)
    {
      return VoidResult::err(
          Error::invalid_state(
              "SDK client is not open"));
    }

    if (!store_ || !sync_ || !scheduler_)
    {
      return VoidResult::err(
          Error::invalid_state(
              "SDK runtime is not initialized"));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::require_transport() const
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    if (!options_.enable_transport)
    {
      return VoidResult::err(
          Error::transport_error(
              "transport is disabled"));
    }

    if (!transport_)
    {
      return VoidResult::err(
          Error::transport_error(
              "transport is not initialized"));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::require_discovery() const
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    if (!options_.enable_discovery)
    {
      return VoidResult::err(
          Error::discovery_error(
              "discovery is disabled"));
    }

    if (!discovery_)
    {
      return VoidResult::err(
          Error::discovery_error(
              "discovery is not initialized"));
    }

    return VoidResult::ok();
  }

  Client::VoidResult Client::require_metadata() const
  {
    auto open_result = require_open();

    if (open_result.is_err())
    {
      return open_result;
    }

    if (!metadata_)
    {
      return VoidResult::err(
          Error::metadata_error(
              "metadata service is not initialized"));
    }

    return VoidResult::ok();
  }

  void Client::build_runtime()
  {
    auto store_config = options_.to_store_config();
    auto sync_config = options_.to_sync_config();

    store_ =
        std::make_unique<store_engine::StoreEngine>(
            std::move(store_config));

    sync_ =
        std::make_unique<sync_engine::SyncEngine>(
            std::move(sync_config));

    scheduler_ =
        std::make_unique<sync_scheduler::SyncScheduler>(
            *sync_);

    if (options_.enable_transport)
    {
      transport_config_ =
          std::make_unique<transport_core::TransportConfig>(
              options_.to_transport_config());

      transport_context_ =
          std::make_unique<transport_core::TransportContext>(
              *transport_config_,
              *sync_);

      transport_ =
          std::make_unique<transport_engine::TransportEngine>(
              *transport_context_);
    }

    if (options_.enable_discovery)
    {
      discovery_ =
          std::make_unique<discovery_api::DiscoveryService>(
              options_.to_discovery_options());
    }

    metadata_ =
        std::make_unique<metadata_api::MetadataService>(
            options_.to_metadata_options());
  }

} // namespace softadastra::sdk

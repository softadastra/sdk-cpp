# Sync

The Softadastra C++ SDK exposes a small sync API through `softadastra::sdk::Client`.

The goal is to let applications work locally first, then synchronize when transport, discovery, or another delivery mechanism is available.

The SDK hides the internal sync wiring behind a simple public surface:

```cpp
client.put("key", "value");
auto state = client.sync_state();
auto tick  = client.tick();
```

## Goal

The sync layer exists to make local-first applications reliable. A local operation should be able to:

- update local state
- be persisted if WAL is enabled
- be registered as a sync operation
- be queued for delivery
- be retried if delivery fails
- eventually converge with other peers

The SDK should make this usable without forcing application developers to manually wire `SyncEngine`, `SyncScheduler`, `Outbox`, or `TransportEngine`.

## Main SDK types

```cpp
#include <softadastra/sdk.hpp>
```

- `softadastra::sdk::Client`
- `softadastra::sdk::ClientOptions`
- `softadastra::sdk::SyncResult`
- `softadastra::sdk::TickResult`
- `softadastra::sdk::Result`
- `softadastra::sdk::Error`

## Basic sync flow

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::persistent(
        "node-sync",
        "data/sync.wal");

    Client client{options};

    auto open_result = client.open();

    if (open_result.is_err())
    {
        std::cerr << open_result.error().message() << "\n";
        return 1;
    }

    auto put_result = client.put("profile/name", "Ada");

    if (put_result.is_err())
    {
        std::cerr << put_result.error().message() << "\n";
        return 1;
    }

    auto tick_result = client.tick();

    if (tick_result.is_err())
    {
        std::cerr << tick_result.error().message() << "\n";
        return 1;
    }

    std::cout << "batch size: " << tick_result.value().batch_size << "\n";

    client.close();

    return 0;
}
```

## Local-first behavior

Softadastra sync starts from a simple rule:

> *local write first*

A write like `client.put("profile/name", "Ada")` should not require the network. The write can succeed locally even when: the device is offline, no peer is reachable, discovery is disabled, transport is not running, or the remote side is temporarily unavailable.

The sync layer exists to move the operation later.

## Sync state

Use `sync_state()` to inspect the current sync pipeline.

```cpp
auto result = client.sync_state();

if (result.is_ok())
{
    const auto &state = result.value();

    std::cout << "outbox:    " << state.outbox_size     << "\n";
    std::cout << "queued:    " << state.queued_count     << "\n";
    std::cout << "in flight: " << state.in_flight_count  << "\n";
    std::cout << "failed:    " << state.failed_count     << "\n";
}
```

`sync_state()` returns `Result<SyncResult, Error>`.

## `SyncResult`

Public SDK snapshot of sync state. Fields:

```cpp
std::size_t outbox_size;
std::size_t queued_count;
std::size_t in_flight_count;
std::size_t acknowledged_count;
std::size_t failed_count;

std::uint64_t last_submitted_version;
std::uint64_t last_applied_remote_version;
std::uint64_t total_retries;
```

Helpers:

```cpp
state.value().has_queued();
state.value().has_in_flight();
state.value().has_failed();
state.value().has_work();
state.value().empty();
state.value().is_valid();
```

## Manual sync tick

```cpp
auto result = client.tick();
```

A tick drives the sync pipeline once. It can: retry expired operations, collect the next batch ready for transport, and optionally prune completed entries.

```cpp
auto result = client.tick();

if (result.is_ok())
{
    const auto &tick = result.value();

    std::cout << "retried: " << tick.retried_count << "\n";
    std::cout << "batch:   " << tick.batch_size    << "\n";
}
```

## `TickResult`

Fields:

```cpp
std::size_t retried_count;
std::size_t pruned_count;
std::size_t batch_size;
```

Helpers:

```cpp
tick.has_work();
tick.empty();
tick.is_valid();
```

## Pruning completed operations

```cpp
auto result = client.tick(true);  // prune_completed = true

if (result.is_ok())
{
    std::cout << "pruned: " << result.value().pruned_count << "\n";
}
```

## Sync without transport

Sync can still be useful without transport.

```cpp
ClientOptions options = ClientOptions::memory_only("node-local");

options.enable_transport = false;
options.enable_discovery = false;
```

In this mode, `put()` can still update local state and the sync pipeline can still track local work. Transport delivery is simply disabled.

Useful for: tests, deterministic local demos, offline-only applications, and applications that provide their own transport later.

## Sync with WAL

For durable sync, enable WAL:

```cpp
ClientOptions options = ClientOptions::persistent(
    "node-durable",
    "data/sdk-sync.wal");
```

This enables:

```cpp
options.enable_wal = true;
options.auto_flush = true;
```

> Persist before relying on network delivery.

## Sync with transport

```cpp
ClientOptions options = ClientOptions::local("node-a");

options.enable_transport = true;
options.transport_host   = "127.0.0.1";
options.transport_port   = 4041;

Client client{options};

auto open_result = client.open();

// Start transport
auto transport_result = client.start_transport();

if (transport_result.is_err())
{
    std::cerr << transport_result.error().message() << "\n";
}

// Connect to a peer
Peer peer{"node-b", "127.0.0.1", 4042};

auto connect_result = client.connect(peer);

// Write and tick
client.put("message", "hello from node-a");

auto tick_result = client.tick();
```

## Peer sync example

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::local("node-a");

    options.enable_wal       = true;
    options.wal_path         = "data/node-a.wal";
    options.auto_flush       = true;

    options.enable_transport = true;
    options.transport_host   = "127.0.0.1";
    options.transport_port   = 4041;

    options.enable_discovery = false;

    Client client{options};

    if (client.open().is_err())
    {
        return 1;
    }

    auto transport = client.start_transport();

    if (transport.is_err())
    {
        std::cerr << transport.error().message() << "\n";
        return 1;
    }

    Peer peer{"node-b", "127.0.0.1", 4042};

    auto connected = client.connect(peer);

    if (connected.is_err())
    {
        std::cout << "peer unavailable: "
                  << connected.error().message() << "\n";
    }

    client.put("sync/message", "hello");

    auto tick = client.tick();

    if (tick.is_ok())
    {
        std::cout << "batch: " << tick.value().batch_size << "\n";
    }

    client.close();

    return 0;
}
```

## Sync with discovery

```cpp
ClientOptions options = ClientOptions::local("node-discovery");

options.enable_discovery         = true;
options.discovery_host           = "127.0.0.1";
options.discovery_port           = 5051;
options.discovery_broadcast_host = "127.0.0.1";
options.discovery_broadcast_port = 5052;

// Start
auto result = client.start_discovery();

// Read peers
auto peers = client.peers();

if (peers.is_ok())
{
    for (const auto &peer : peers.value())
    {
        std::cout << peer.node_id << " "
                  << peer.host   << ":"
                  << peer.port   << "\n";
    }
}

// Connect to all discovered peers
for (const auto &peer : peers.value())
{
    client.connect(peer);
}
```

## Sync statuses

Internally, Softadastra sync tracks statuses: `pending`, `queued`, `in_flight`, `acknowledged`, `applied`, `failed`. For most application code, this is enough:

```cpp
auto state = client.sync_state();

if (state.value().has_work())
{
    client.tick();
}
```

## Acknowledgements

When acknowledgement tracking is enabled:

```cpp
options.require_ack    = true;
options.ack_timeout_ms = 10000;
```

A sync operation lifecycle:

```
queued → in_flight → acknowledged → applied
```

If an acknowledgement does not arrive in time, the operation is retried.

## Retries

```cpp
options.max_retries      = 5;
options.retry_interval_ms = 5000;
```

The sync scheduler checks expired operations during `tick()`.

```cpp
auto tick = client.tick();

if (tick.is_ok())
{
    std::cout << "retried: " << tick.value().retried_count << "\n";
}
```

## Conflict policy

```cpp
options.conflict_policy = ConflictPolicy::LastWriteWins;
```

The first SDK version maps to the internal default: `last_write_wins`. Later SDK versions can expose richer conflict APIs without changing the basic client flow.

## Offline-first rule

A failed network send must not destroy the local operation. The correct behavior:

- local state remains valid
- sync operation remains tracked
- retry can happen later
- network failure is reported separately

This is why the SDK separates `client.put(...)` from `client.tick()`, `client.start_transport()`, and `client.connect(...)`.

## Recommended sync loop

```cpp
while (running)
{
    auto tick = client.tick(true);

    if (tick.is_err())
    {
        std::cerr << tick.error().message() << "\n";
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

Manual ticking is preferred over a hidden background thread because it is deterministic, easier to test, easier to debug, and safer for embedded applications.

## Recommended SDK public methods

```cpp
Result<void, Error>  open();
void                 close();

Result<void, Error>  put(Key key, Value value);
Result<Value, Error> get(Key key);
Result<void, Error>  remove(Key key);

Result<SyncResult, Error> sync_state() const;
Result<TickResult, Error> tick(bool prune_completed = false);

Result<void, Error>  start_transport();
void                 stop_transport();

Result<void, Error>  connect(const Peer &peer);

Result<void, Error>               start_discovery();
void                              stop_discovery();
Result<std::vector<Peer>, Error>  peers() const;
```

## What the SDK should hide

Application developers should not need to manually create:

- `softadastra::store::engine::StoreEngine`
- `softadastra::sync::engine::SyncEngine`
- `softadastra::sync::scheduler::SyncScheduler`
- `softadastra::transport::engine::TransportEngine`
- `softadastra::discovery::DiscoveryService`
- `softadastra::metadata::MetadataService`

The SDK client owns or wires these internally.

## Example: local write, inspect sync state, tick

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    Client client{ClientOptions::persistent(
        "node-sync-demo",
        "data/sync-demo.wal")};

    if (client.open().is_err())
    {
        return 1;
    }

    client.put("doc/title", "Offline-first note");

    auto state = client.sync_state();

    if (state.is_ok())
    {
        std::cout << "outbox before tick: "
                  << state.value().outbox_size << "\n";
    }

    auto tick = client.tick();

    if (tick.is_ok())
    {
        std::cout << "batch produced: "
                  << tick.value().batch_size << "\n";
    }

    client.close();

    return 0;
}
```

## Example: retry and prune

```cpp
auto tick = client.tick(true);

if (tick.is_ok())
{
    std::cout << "retried: " << tick.value().retried_count << "\n";
    std::cout << "pruned:  " << tick.value().pruned_count  << "\n";
    std::cout << "batch:   " << tick.value().batch_size    << "\n";
}
```

## Summary

The SDK sync layer exposes this simple mental model:

```
write locally
persist locally
track operation
tick sync
send when possible
retry when needed
converge later
```

At the API level, the developer should mostly see:

```cpp
client.put("key", "value");
client.sync_state();
client.tick();
```

> A clean, deterministic public API over Softadastra's offline-first sync foundation.

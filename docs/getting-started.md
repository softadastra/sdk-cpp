# Getting Started with the Softadastra C++ SDK

The Softadastra C++ SDK provides a small, stable, user-facing API on top of the Softadastra runtime modules.

It is designed to make Softadastra easy to embed in C++ applications without exposing the full internal module graph directly.

## What the SDK provides

The SDK wraps the lower-level Softadastra modules:

- `core`
- `store`
- `wal`
- `sync`
- `transport`
- `discovery`
- `metadata`

Instead of manually wiring these modules together, applications use one main entry point:

```cpp
#include <softadastra/sdk.hpp>

softadastra::sdk::Client client;
```

## Main idea

Softadastra is built around local-first and offline-first execution. That means:

- data is written locally first
- local writes can be persisted through the WAL
- sync operations are tracked
- transport and discovery can be enabled when peer communication is needed
- metadata exposes local node information

The SDK keeps this model simple.

## Basic local usage

The simplest usage is an in-memory local client.

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::memory_only("node-local");

    Client client{options};

    auto open_result = client.open();

    if (open_result.is_err())
    {
        std::cerr << open_result.error().message() << "\n";
        return 1;
    }

    auto put_result = client.put("app/name", "Softadastra SDK");

    if (put_result.is_err())
    {
        std::cerr << put_result.error().message() << "\n";
        return 1;
    }

    auto value_result = client.get("app/name");

    if (value_result.is_err())
    {
        std::cerr << value_result.error().message() << "\n";
        return 1;
    }

    std::cout << value_result.value().to_string() << "\n";

    client.close();

    return 0;
}
```

## Persistent local store

To persist local operations through the WAL, enable WAL support.

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::persistent(
        "node-persistent",
        "data/sdk-store.wal");

    Client client{options};

    auto open_result = client.open();

    if (open_result.is_err())
    {
        std::cerr << open_result.error().message() << "\n";
        return 1;
    }

    auto put_result = client.put("settings/theme", "dark");

    if (put_result.is_err())
    {
        std::cerr << put_result.error().message() << "\n";
        return 1;
    }

    auto value_result = client.get("settings/theme");

    if (value_result.is_ok())
    {
        std::cout << value_result.value().to_string() << "\n";
    }

    client.close();

    return 0;
}
```

## Client options

`ClientOptions` controls how the SDK client is initialized.

```cpp
ClientOptions options;

options.node_id      = "node-1";
options.display_name = "Local Node";
options.version      = "0.1.0";

options.enable_wal = true;
options.wal_path   = "data/store.wal";
options.auto_flush = true;

options.enable_transport = true;
options.transport_host   = "127.0.0.1";
options.transport_port   = 4040;

options.enable_discovery        = true;
options.discovery_host          = "127.0.0.1";
options.discovery_port          = 5050;
```

### Recommended constructors

| Constructor | Use case |
|-------------|----------|
| `ClientOptions::memory_only("node")` | Tests, demos, temporary state |
| `ClientOptions::local("node")` | Local development, optional transport/discovery |
| `ClientOptions::persistent("node", "path/store.wal")` | Durable writes, offline-first apps |

## Opening and closing the client

A client must be opened before use.

```cpp
Client client{options};

auto result = client.open();

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
    return 1;
}

// ... use client ...

client.close();
```

> The destructor also closes the client automatically, but explicit `close()` is clearer in examples and applications.

## Writing values

```cpp
// String shorthand
auto result = client.put("profile/name", "Ada");

// Typed SDK values
Key   key{"profile/name"};
Value value = Value::from_string("Ada");

auto result = client.put(key, value);
```

## Reading values

```cpp
auto result = client.get("profile/name");

if (result.is_ok())
{
    std::cout << result.value().to_string() << "\n";
}
else
{
    std::cerr << result.error().message() << "\n";
}
```

## Removing values

```cpp
auto result = client.remove("profile/name");

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
}
```

## Checking keys

```cpp
if (client.contains("profile/name"))
{
    std::cout << "key exists\n";
}

std::cout << client.size() << "\n";

if (client.empty())
{
    std::cout << "store is empty\n";
}
```

## Sync state

Every local operation can be submitted into the sync pipeline.

```cpp
auto state = client.sync_state();

if (state.is_ok())
{
    std::cout << "outbox: " << state.value().outbox_size  << "\n";
    std::cout << "queued: " << state.value().queued_count << "\n";
}
```

## Manual sync tick

The SDK exposes deterministic manual sync ticking.

```cpp
auto result = client.tick();

if (result.is_ok())
{
    std::cout << "batch size: " << result.value().batch_size << "\n";
}
```

A tick can:

- retry expired operations
- return the next batch ready for transport
- optionally prune completed operations

```cpp
auto result = client.tick(true);
```

## Transport

Enable transport to communicate with peers:

```cpp
ClientOptions options = ClientOptions::local("node-a");

options.enable_transport = true;
options.transport_host   = "127.0.0.1";
options.transport_port   = 4041;

// Start
auto result = client.start_transport();

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
}

// Connect to a peer
Peer peer{"node-b", "127.0.0.1", 4042};

auto result = client.connect(peer);

// Stop
client.stop_transport();
```

## Discovery

Enable discovery to find other peers:

```cpp
ClientOptions options = ClientOptions::local("node-discovery");

options.enable_discovery          = true;
options.discovery_host            = "127.0.0.1";
options.discovery_port            = 5051;
options.discovery_broadcast_host  = "127.0.0.1";
options.discovery_broadcast_port  = 5052;

// Start
auto result = client.start_discovery();

// List peers
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

// Stop
client.stop_discovery();
```

## Metadata

The SDK exposes local node metadata through `NodeInfo`.

```cpp
auto result = client.refresh_node_info();

if (result.is_ok())
{
    const auto &node = result.value();

    std::cout << node.node_id      << "\n";
    std::cout << node.display_name << "\n";
    std::cout << node.hostname     << "\n";
    std::cout << node.os_name      << "\n";
    std::cout << node.version      << "\n";
}
```

## Error handling

SDK operations return `Result<T, Error>` or `Result<void, Error>`.

```cpp
auto result = client.get("missing/key");

if (result.is_err())
{
    std::cerr << result.error().code_string()
              << ": "
              << result.error().message()
              << "\n";
}
```

Common error categories:

- `invalid argument`
- `invalid state`
- `not found`
- `store error`
- `sync error`
- `transport error`
- `discovery error`
- `metadata error`
- `internal error`

## Building

```bash
# Configure
cmake --preset dev-ninja

# Build
cmake --build --preset build-ninja

# Run tests
ctest --test-dir build-ninja --output-on-failure
```

## Example files

| File | Description |
|------|-------------|
| `examples/01_local_store.cpp` | Basic in-memory local store |
| `examples/02_persistent_store.cpp` | WAL-backed persistent store |
| `examples/03_remove_value.cpp` | Removing keys |
| `examples/04_basic_sync.cpp` | Local sync pipeline |
| `examples/05_tcp_peer_sync.cpp` | TCP peer sync |
| `examples/06_discovery.cpp` | Peer discovery |
| `examples/07_node_metadata.cpp` | Node metadata |

### Recommended first test

```bash
cmake --build --preset build-ninja --target sdk_cpp_example_01_local_store
```

Then run the generated example from the build directory.

## SDK design rule

The SDK should remain the clean public API.

Internal modules can stay powerful and detailed, but application developers should not need to manually understand `StoreEngine`, `SyncEngine`, `SyncScheduler`, `TransportEngine`, `DiscoveryService`, or `MetadataService`.

The SDK client hides that wiring behind:

```cpp
Client client{options};
client.open();
client.put("key", "value");
client.tick();
client.close();
```

> That is the main purpose of `sdk-cpp`.

# Metadata

The Softadastra C++ SDK exposes node metadata through `softadastra::sdk::Client`.

Metadata gives applications a simple way to inspect the current Softadastra node:

```cpp
auto info = client.node_info();
```

It hides the lower-level metadata module and exposes a clean public structure: `softadastra::sdk::NodeInfo`.

---

## Goal

Metadata answers one question:

> *Who is this Softadastra node, and what can it do?*

A Softadastra node may expose: node id, display name, hostname, operating system, runtime version, uptime, and supported capabilities.

Useful for: dashboards, CLI status commands, peer diagnostics, discovery announcements, logs, monitoring, and debugging local-first systems.

---

## Main SDK types

```cpp
#include <softadastra/sdk.hpp>
```

- `softadastra::sdk::Client`
- `softadastra::sdk::ClientOptions`
- `softadastra::sdk::NodeInfo`
- `softadastra::sdk::Result`
- `softadastra::sdk::Error`

---

## Basic metadata flow

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::local("node-metadata");

    options.display_name = "Local Metadata Node";
    options.version      = "0.1.0";

    Client client{options};

    auto open_result = client.open();

    if (open_result.is_err())
    {
        std::cerr << open_result.error().message() << "\n";
        return 1;
    }

    auto info_result = client.node_info();

    if (info_result.is_err())
    {
        std::cerr << info_result.error().message() << "\n";
        client.close();
        return 1;
    }

    const auto &info = info_result.value();

    std::cout << "node id      : " << info.node_id      << "\n";
    std::cout << "display name : " << info.display_name << "\n";
    std::cout << "hostname     : " << info.hostname      << "\n";
    std::cout << "os           : " << info.os_name       << "\n";
    std::cout << "version      : " << info.version       << "\n";
    std::cout << "uptime ms    : " << info.uptime_ms     << "\n";

    client.close();

    return 0;
}
```

---

## `ClientOptions` fields

```cpp
ClientOptions options = ClientOptions::local("node-a");

options.display_name = "Node A";
options.version      = "0.1.0";
```

Expected fields:

```cpp
std::string node_id;       // required
std::string display_name;  // optional (falls back to node_id)
std::string version;       // optional (SDK provides default)
```

All client modes support metadata as long as `node_id` is valid:

```cpp
ClientOptions::memory_only("node-memory");
ClientOptions::local("node-local");
ClientOptions::persistent("node-persistent", "data/sdk-store.wal");
```

---

## Reading node metadata

```cpp
auto result = client.node_info();

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

`node_info()` returns `Result<NodeInfo, Error>`.

---

## Refreshing node metadata

`refresh_node_info()` updates runtime fields such as uptime before returning.

```cpp
auto result = client.refresh_node_info();

if (result.is_ok())
{
    std::cout << "uptime: " << result.value().uptime_ms << "ms\n";
}
```

Recommended behavior:

```
node_info()         →  returns current metadata snapshot
refresh_node_info() →  refreshes runtime info, returns updated snapshot
```

Example:

```cpp
auto before = client.node_info();

std::this_thread::sleep_for(std::chrono::seconds(1));

auto after = client.refresh_node_info();

if (before.is_ok() && after.is_ok())
{
    std::cout << "before: " << before.value().uptime_ms << "\n";
    std::cout << "after:  " << after.value().uptime_ms  << "\n";
}
```

---

## `NodeInfo` type

Simple public SDK structure.

```cpp
std::string node_id;
std::string display_name;
std::string hostname;
std::string os_name;
std::string version;

std::int64_t started_at_ms;
std::int64_t uptime_ms;

std::vector<std::string> capabilities;
```

Helpers:

```cpp
node.is_valid();
node.valid();
node.label();           // display_name if set, otherwise node_id
node.uptime_seconds();
node.has_capability("sync");
node.has_capability("transport");
```

### Full declaration

```cpp
namespace softadastra::sdk
{
    struct NodeInfo
    {
        std::string node_id{};
        std::string display_name{};
        std::string hostname{};
        std::string os_name{};
        std::string version{};

        std::int64_t started_at_ms{0};
        std::int64_t uptime_ms{0};

        std::vector<std::string> capabilities{};

        [[nodiscard]] bool is_valid() const noexcept
        {
            return !node_id.empty()   &&
                   !hostname.empty()  &&
                   !os_name.empty()   &&
                   !version.empty()   &&
                   started_at_ms > 0;
        }

        [[nodiscard]] bool valid() const noexcept
        {
            return is_valid();
        }

        [[nodiscard]] const std::string &label() const noexcept
        {
            return display_name.empty() ? node_id : display_name;
        }

        [[nodiscard]] double uptime_seconds() const noexcept
        {
            return static_cast<double>(uptime_ms) / 1000.0;
        }

        [[nodiscard]] bool has_capability(const std::string &value) const
        {
            for (const auto &capability : capabilities)
            {
                if (capability == value) return true;
            }
            return false;
        }

        void clear()
        {
            node_id.clear();
            display_name.clear();
            hostname.clear();
            os_name.clear();
            version.clear();
            started_at_ms = 0;
            uptime_ms     = 0;
            capabilities.clear();
        }
    };
}
```

---

## Capabilities

Common capabilities:

- `core`, `fs`, `wal`, `store`, `sync`
- `transport`, `discovery`, `metadata`, `app`, `cli`

```cpp
auto result = client.node_info();

if (result.is_ok())
{
    const auto &node = result.value();

    if (node.has_capability("sync"))
    {
        std::cout << "sync is available\n";
    }

    if (node.has_capability("transport"))
    {
        std::cout << "transport is available\n";
    }
}
```

---

## Metadata and local store

Metadata identifies the node. The store contains application data. They are independent.

```cpp
client.put("profile/name", "Ada");   // store
auto info = client.node_info();      // metadata — no conflict
```

---

## Metadata and sync

The SDK keeps node identity consistent across metadata, sync, transport, and discovery.

```cpp
ClientOptions options = ClientOptions::persistent("node-sync", "data/sync.wal");
options.display_name  = "Sync Node";
```

The sync layer uses `node-sync` as the local node identity.

---

## Metadata and transport

```cpp
ClientOptions options = ClientOptions::local("node-a");
options.display_name  = "Node A";

options.enable_transport = true;
options.transport_host   = "127.0.0.1";
options.transport_port   = 4041;
```

The SDK can expose the node as `node-a at 127.0.0.1:4041`.

---

## Metadata and discovery

Discovery announcements map metadata internally. Public code stays simple:

```cpp
client.start_discovery();
auto peers = client.peers();
```

Later SDK versions can expose richer peer metadata.

---

## Metadata disabled behavior

For the first SDK version, metadata is not optional. Every opened client should have local metadata.

```cpp
auto result = client.node_info();
// works after client.open() as long as ClientOptions is valid
```

If the client is closed, `node_info()` returns an error:

```cpp
Client client;

auto result = client.node_info();

if (result.is_err())
{
    std::cout << "client is not open\n";
}
```

---

## Error handling

```cpp
auto result = client.node_info();

if (result.is_err())
{
    const auto &error = result.error();

    std::cerr << error.code_string()
              << ": "
              << error.message()
              << "\n";
}
```

Common metadata errors: `invalid_state`, `invalid_argument`, `metadata_error`, `internal_error`.

---

## Recommended SDK metadata API

```cpp
Result<NodeInfo, Error> node_info() const;
Result<NodeInfo, Error> refresh_node_info();
```

That is enough for the first public SDK.

---

## What the SDK should hide

Application developers should not need to manually use:

- `softadastra::metadata::core::MetadataConfig`
- `softadastra::metadata::core::NodeMetadata`
- `softadastra::metadata::core::NodeIdentity`
- `softadastra::metadata::core::NodeRuntimeInfo`
- `softadastra::metadata::core::NodeCapabilities`
- `softadastra::metadata::engine::MetadataEngine`
- `softadastra::metadata::MetadataService`
- `softadastra::metadata::registry::MetadataRegistry`

---

## Example: print node metadata

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::local("node-info-demo");
    options.display_name  = "Node Info Demo";
    options.version       = "0.1.0";

    Client client{options};

    if (client.open().is_err())
    {
        return 1;
    }

    auto info = client.refresh_node_info();

    if (info.is_err())
    {
        std::cerr << info.error().message() << "\n";
        return 1;
    }

    const auto &node = info.value();

    std::cout << "Node\n";
    std::cout << "  id         : " << node.node_id      << "\n";
    std::cout << "  label      : " << node.label()      << "\n";
    std::cout << "  hostname   : " << node.hostname      << "\n";
    std::cout << "  os         : " << node.os_name       << "\n";
    std::cout << "  version    : " << node.version       << "\n";
    std::cout << "  started at : " << node.started_at_ms << "\n";
    std::cout << "  uptime ms  : " << node.uptime_ms     << "\n";

    std::cout << "Capabilities\n";

    for (const auto &capability : node.capabilities)
    {
        std::cout << "  - " << capability << "\n";
    }

    client.close();

    return 0;
}
```

---

## Example: check runtime capabilities

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::persistent(
        "node-capabilities",
        "data/capabilities.wal");

    options.enable_transport = true;
    options.transport_host   = "127.0.0.1";
    options.transport_port   = 4041;

    options.enable_discovery = true;
    options.discovery_host   = "127.0.0.1";
    options.discovery_port   = 5051;

    Client client{options};

    if (client.open().is_err())
    {
        return 1;
    }

    auto info = client.node_info();

    if (info.is_ok())
    {
        const auto &node = info.value();

        if (node.has_capability("store"))     std::cout << "store available\n";
        if (node.has_capability("sync"))      std::cout << "sync available\n";
        if (node.has_capability("transport")) std::cout << "transport available\n";
        if (node.has_capability("discovery")) std::cout << "discovery available\n";
    }

    client.close();

    return 0;
}
```

---

## Metadata in dashboards

A dashboard can display: node id, display name, hostname, OS, version, uptime, capabilities, transport status, discovery status, and sync state. The metadata API gives the identity part — sync, transport, and discovery APIs provide the runtime state.

---

## Metadata in CLI tools

```cpp
auto info = client.node_info();
// format result as a table for softadastra node info / status / peers
```

---

## Design rule

The SDK should expose a public `NodeInfo` object, not the internal `NodeMetadata` object. The public object should be easy to serialize into: JSON, CLI output, dashboard API responses, logs, and diagnostics.

---

## Summary

The developer should mostly write:

```cpp
auto info = client.node_info();

std::cout << info.value().node_id  << "\n";
std::cout << info.value().hostname << "\n";
std::cout << info.value().version  << "\n";
```

The public API stays small:

```cpp
client.node_info();
client.refresh_node_info();
```

> That is enough for the first version of `sdk-cpp`.

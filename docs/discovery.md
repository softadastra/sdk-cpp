# Discovery

The Softadastra C++ SDK exposes peer discovery through `softadastra::sdk::Client`.

Discovery is used to find other Softadastra nodes without hardcoding every peer manually.

At the SDK level, the goal is simple:

```cpp
client.start_discovery();
auto peers = client.peers();
client.connect(peer);
```

The SDK hides the lower-level discovery module behind a small public API.

## Goal

Discovery answers one question:

> *Which Softadastra peers are available near me?*

In a local-first and offline-first system, this matters because peers may appear and disappear depending on: network availability, LAN conditions, device restarts, mobile devices moving between networks, temporary outages, or local edge nodes joining or leaving.

## Main SDK types

```cpp
#include <softadastra/sdk.hpp>
```

- `softadastra::sdk::Client`
- `softadastra::sdk::ClientOptions`
- `softadastra::sdk::Peer`
- `softadastra::sdk::Result`
- `softadastra::sdk::Error`

## Basic discovery flow

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::local("node-discovery");

    options.enable_discovery         = true;
    options.discovery_host           = "127.0.0.1";
    options.discovery_port           = 5051;
    options.discovery_broadcast_host = "127.0.0.1";
    options.discovery_broadcast_port = 5052;

    Client client{options};

    auto open_result = client.open();

    if (open_result.is_err())
    {
        std::cerr << open_result.error().message() << "\n";
        return 1;
    }

    auto discovery_result = client.start_discovery();

    if (discovery_result.is_err())
    {
        std::cerr << discovery_result.error().message() << "\n";
        client.close();
        return 1;
    }

    auto peers_result = client.peers();

    if (peers_result.is_ok())
    {
        for (const auto &peer : peers_result.value())
        {
            std::cout << peer.node_id << " "
                      << peer.host   << ":"
                      << peer.port   << "\n";
        }
    }

    client.stop_discovery();
    client.close();

    return 0;
}
```

## `ClientOptions` fields

```cpp
ClientOptions options = ClientOptions::local("node-a");

options.enable_discovery         = true;
options.discovery_host           = "127.0.0.1";
options.discovery_port           = 5051;
options.discovery_broadcast_host = "127.0.0.1";
options.discovery_broadcast_port = 5052;
```

Expected fields:

```cpp
bool          enable_discovery;

std::string   discovery_host;
std::uint16_t discovery_port;

std::string   discovery_broadcast_host;
std::uint16_t discovery_broadcast_port;
```

## Enabling discovery

Discovery is disabled in memory-only mode. To use it, enable explicitly:

```cpp
ClientOptions options = ClientOptions::local("node-discovery");

options.enable_discovery = true;
options.discovery_host   = "127.0.0.1";
options.discovery_port   = 5051;
```

## Starting discovery

```cpp
auto result = client.start_discovery();

if (result.is_err())
{
    std::cerr << result.error().code_string()
              << ": "
              << result.error().message()
              << "\n";
}
```

If discovery is disabled in options, `start_discovery()` returns an error.

## Checking discovery state

```cpp
if (client.discovery_running())
{
    std::cout << "discovery is running\n";
}
```

## Stopping discovery

```cpp
client.stop_discovery();
```

Safe to call even if discovery is already stopped.

## Listing discovered peers

`peers()` returns `Result<std::vector<Peer>, Error>`.

```cpp
auto result = client.peers();

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
    return 1;
}

for (const auto &peer : result.value())
{
    std::cout << peer.node_id << " "
              << peer.host   << ":"
              << peer.port   << "\n";
}
```

## `Peer` type

Simple public SDK peer object.

```cpp
std::string   node_id;
std::string   host;
std::uint16_t port;
std::int64_t  last_seen_at_ms;
```

Helpers:

```cpp
peer.is_valid();
peer.valid();
peer.is_localhost();
peer.endpoint();
```

## Discovery and transport

Discovery only finds peers. Transport connects to peers. The usual flow:

```
discovery finds peer
  →  transport connects to peer
  →  sync sends operations
```

Responsibilities:

```
Discovery  →  who exists?
Transport  →  how do we connect?
Sync       →  what do we send?
Store      →  what local state do we keep?
```

At SDK level:

```cpp
auto peers = client.peers();

if (peers.is_ok())
{
    for (const auto &peer : peers.value())
    {
        client.connect(peer);
    }
}
```

## Connecting to discovered peers

Requires transport to be enabled and started.

```cpp
options.enable_transport = true;
options.transport_port   = 4041;

client.start_transport();

auto peers_result = client.peers();

if (peers_result.is_ok())
{
    for (const auto &peer : peers_result.value())
    {
        auto connect_result = client.connect(peer);

        if (connect_result.is_err())
        {
            std::cerr << "failed to connect to "
                      << peer.node_id << ": "
                      << connect_result.error().message() << "\n";
        }
    }
}
```

## Discovery with sync

A complete flow:

```cpp
client.open();

client.start_discovery();
client.start_transport();

auto peers = client.peers();

if (peers.is_ok())
{
    for (const auto &peer : peers.value())
    {
        client.connect(peer);
    }
}

client.put("message", "hello");
client.tick();

client.close();
```

## Discovery with local writes

Local writes must not depend on discovery.

```cpp
client.put("profile/name", "Ada");  // works even if discovery is disabled
```

Discovery is only an optimization for finding peers later. The offline-first rule remains: local work must continue without network discovery.

## Discovery failure behavior

If discovery cannot start, local store and sync should still be usable.

```cpp
auto discovery = client.start_discovery();

if (discovery.is_err())
{
    std::cerr << "discovery unavailable: "
              << discovery.error().message() << "\n";
}

client.put("local/key", "still works");
```

## Discovery disabled behavior

When `enable_discovery = false`, these return errors:

```cpp
client.start_discovery();
client.peers();
```

This makes configuration mistakes visible.

```cpp
ClientOptions options = ClientOptions::memory_only("node-memory");

Client client{options};
client.open();

auto result = client.start_discovery();

if (result.is_err())
{
    std::cout << "discovery disabled\n";
}
```

## Peer expiration

Internally, discovery tracks peer states: `discovered`, `alive`, `stale`, `expired`. The public SDK exposes only valid usable peers:

```cpp
auto peers = client.peers();
```

Advanced diagnostics APIs can be added in later versions.

## Discovery service internals

Internally, the SDK can use `DiscoveryService`, `DiscoveryEngine`, `UdpDiscoveryBackend`, `DiscoveryRegistry` — but the SDK user should not need to create these manually. The public API stays:

```cpp
client.start_discovery();
client.peers();
client.stop_discovery();
```

## Example: print discovered peers

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::local("node-discovery-demo");

    options.enable_discovery         = true;
    options.discovery_host           = "127.0.0.1";
    options.discovery_port           = 5051;
    options.discovery_broadcast_host = "127.0.0.1";
    options.discovery_broadcast_port = 5052;
    options.enable_transport         = false;

    Client client{options};

    if (client.open().is_err())
    {
        return 1;
    }

    auto started = client.start_discovery();

    if (started.is_err())
    {
        std::cerr << started.error().message() << "\n";
        return 1;
    }

    auto peers = client.peers();

    if (peers.is_ok())
    {
        std::cout << "peers found: " << peers.value().size() << "\n";

        for (const auto &peer : peers.value())
        {
            std::cout << "  " << peer.node_id << " "
                              << peer.host    << ":"
                              << peer.port    << "\n";
        }
    }

    client.stop_discovery();
    client.close();

    return 0;
}
```

## Example: discover then connect

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::local("node-a");

    options.enable_transport         = true;
    options.transport_host           = "127.0.0.1";
    options.transport_port           = 4041;

    options.enable_discovery         = true;
    options.discovery_host           = "127.0.0.1";
    options.discovery_port           = 5051;
    options.discovery_broadcast_host = "127.0.0.1";
    options.discovery_broadcast_port = 5052;

    Client client{options};

    if (client.open().is_err())
    {
        return 1;
    }

    if (client.start_transport().is_err())
    {
        std::cerr << "transport failed\n";
        return 1;
    }

    if (client.start_discovery().is_err())
    {
        std::cerr << "discovery failed\n";
        return 1;
    }

    auto peers = client.peers();

    if (peers.is_ok())
    {
        for (const auto &peer : peers.value())
        {
            auto connected = client.connect(peer);

            if (connected.is_ok())
            {
                std::cout << "connected to " << peer.node_id << "\n";
            }
        }
    }

    client.put("hello", "from node-a");
    client.tick();

    client.close();

    return 0;
}
```

## Recommended SDK discovery API

```cpp
Result<void, Error>              start_discovery();
void                             stop_discovery();
bool                             discovery_running() const noexcept;
Result<std::vector<Peer>, Error> peers() const;
```

## What the SDK should hide

Application developers should not need to manually use:

- `softadastra::discovery::core::DiscoveryConfig`
- `softadastra::discovery::core::DiscoveryContext`
- `softadastra::discovery::engine::DiscoveryEngine`
- `softadastra::discovery::backend::UdpDiscoveryBackend`
- `softadastra::discovery::peer::DiscoveryRegistry`
- `softadastra::discovery::client::DiscoveryClient`
- `softadastra::discovery::server::DiscoveryServer`

## Design rule

Discovery must remain optional. A Softadastra application should be able to work in these modes:

```
local only
local + WAL
local + WAL + sync
local + WAL + sync + transport
local + WAL + sync + transport + discovery
```

Discovery is the last layer, not the foundation. The foundation remains: local writes, durability, sync tracking, safe retry, and eventual convergence.

## Summary

The developer should mostly write:

```cpp
client.start_discovery();

auto peers = client.peers();

for (const auto &peer : peers.value())
{
    client.connect(peer);
}
```

Internally, Softadastra can use UDP discovery, announcements, peer registries, and metadata — but the SDK must keep the public API small, stable, and easy to understand.

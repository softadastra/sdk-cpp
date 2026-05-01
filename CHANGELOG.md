# Changelog

All notable changes to `sdk-cpp` will be documented in this file.

The format follows a simple release-oriented structure.

## v0.1.0 — Initial SDK foundation

### Added

- Initial public C++ SDK repository structure.

- Top-level SDK umbrella header:

```cpp
  #include <softadastra/sdk.hpp>
```

- Public SDK client API foundation:
  - `softadastra::sdk::Client`
  - `softadastra::sdk::ClientOptions`

- Public result and error primitives:
  - `softadastra::sdk::Result<T, E>`
  - `softadastra::sdk::Error`

- Public key-value types:
  - `softadastra::sdk::Key`
  - `softadastra::sdk::Value`

- Public peer and node metadata types:
  - `softadastra::sdk::Peer`
  - `softadastra::sdk::NodeInfo`

- Public sync result types:
  - `softadastra::sdk::SyncResult`
  - `softadastra::sdk::TickResult`

- Local store API draft through `Client`:

```cpp
  client.open();
  client.close();
  client.put("key", "value");
  client.get("key");
  client.remove("key");
  client.contains("key");
```

- Persistent store configuration through `ClientOptions`.
- Memory-only client configuration through `ClientOptions::memory_only()`.
- Local client configuration through `ClientOptions::local()`.
- Persistent client configuration through `ClientOptions::persistent()`.

- Initial sync API draft:

```cpp
  client.sync_state();
  client.tick();
```

- Transport API draft:

```cpp
  client.start_transport();
  client.stop_transport();
  client.transport_running();
  client.connect(peer);
```

- Discovery API draft:

```cpp
  client.start_discovery();
  client.stop_discovery();
  client.discovery_running();
  client.peers();
```

- Metadata API draft:

```cpp
  client.node_info();
  client.refresh_node_info();
```

- Conversion source files for mapping internal Softadastra modules to public SDK types:
  - `src/conversions/StoreConversions.cpp`
  - `src/conversions/SyncConversions.cpp`
  - `src/conversions/TransportConversions.cpp`
  - `src/conversions/DiscoveryConversions.cpp`
  - `src/conversions/MetadataConversions.cpp`

- Initial examples:
  - `examples/01_local_store.cpp`
  - `examples/02_persistent_store.cpp`
  - `examples/03_remove_value.cpp`
  - `examples/04_basic_sync.cpp`
  - `examples/05_tcp_peer_sync.cpp`
  - `examples/06_discovery.cpp`
  - `examples/07_node_metadata.cpp`

- Initial tests:
  - `tests/ResultTests.cpp`
  - `tests/ClientOptionsTests.cpp`
  - `tests/ClientTests.cpp`

- Documentation drafts:
  - `docs/getting-started.md`
  - `docs/local-store.md`
  - `docs/sync.md`
  - `docs/discovery.md`
  - `docs/metadata.md`

### Design

- Established `sdk-cpp` as the public developer-facing C++ SDK for Softadastra.
- Kept the repository name as `sdk-cpp`, without the `softadastra-` prefix.
- Designed the SDK as a stable facade over the internal Softadastra modules: `core`, `fs`, `wal`, `store`, `sync`, `transport`, `discovery`, `metadata`.
- Kept the public API intentionally smaller than the internal module APIs.
- Defined `Client` as the main user-facing entry point.
- Defined `ClientOptions` as the main configuration object.
- Defined `Result<T, Error>` as the public error-handling style.
- Defined simple public DTO-style objects for SDK users: `Key`, `Value`, `Peer`, `NodeInfo`, `SyncResult`, `TickResult`.

### Architecture

- Added CMake project foundation for the SDK.
- Added CMake package config template: `cmake/sdk-cpp-config.cmake.in`
- Prepared the SDK to link against the existing Softadastra foundation libraries.
- Prepared examples and tests to be built through CMake.
- Prepared the SDK for future installation through CMake package exports.

### Notes

This first release is the foundation release.

The goal of `v0.1.0` is not to expose every internal Softadastra capability directly. The goal is to provide a clean and stable first public SDK layer:

```cpp
softadastra::sdk::Client client{
    softadastra::sdk::ClientOptions::persistent(
        "node-1",
        "data/sdk.wal")};

client.open();
client.put("hello", "world");
auto value = client.get("hello");
client.close();
```

> Future releases will expand the SDK while keeping the public API simple and predictable.

# Local Store

The Softadastra C++ SDK exposes a simple local key-value API through `softadastra::sdk::Client`.

The local store is the first layer of the SDK. It lets an application write, read, remove, and inspect local data without manually using the internal `store`, `wal`, or `sync` modules.

## Goal

The local store API should feel simple:

```cpp
client.put("profile/name", "Ada");
auto value = client.get("profile/name");
client.remove("profile/name");
```

Internally, the SDK can use `softadastra::store`, `softadastra::wal`, and `softadastra::sync` — but the user of the SDK should not need to wire these modules manually.

## Main types

```cpp
#include <softadastra/sdk.hpp>
```

Important types:

- `softadastra::sdk::Client`
- `softadastra::sdk::ClientOptions`
- `softadastra::sdk::Key`
- `softadastra::sdk::Value`
- `softadastra::sdk::Result`
- `softadastra::sdk::Error`

## Creating a memory-only store

Use `ClientOptions::memory_only()` when you do not want WAL persistence.

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::memory_only("node-memory");

    Client client{options};

    auto open_result = client.open();

    if (open_result.is_err())
    {
        std::cerr << open_result.error().message() << "\n";
        return 1;
    }

    client.put("app/name", "Softadastra SDK");

    auto value = client.get("app/name");

    if (value.is_ok())
    {
        std::cout << value.value().to_string() << "\n";
    }

    client.close();

    return 0;
}
```

Use memory-only mode for: tests, examples, temporary local state, and demos without persistence.

## Creating a persistent store

Use `ClientOptions::persistent()` when local writes must be backed by WAL persistence.

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

Persistent mode is useful for: offline-first applications, durable local writes, recovery after restart, and sync pipelines backed by WAL.

## Opening the client

The client must be opened before store operations.

```cpp
Client client{options};

auto result = client.open();

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
    return 1;
}

if (client.is_open())
{
    // safe to use
}

// Backward-compatible alias
client.opened();
```

## Closing the client

```cpp
client.close();
```

> The destructor also closes the client, but explicit `close()` is better in examples and tests.

## Writing values

```cpp
// String shorthand
auto result = client.put("profile/name", "Ada");

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
}
```

### Writing with SDK value objects

```cpp
Key   key{"profile/name"};
Value value = Value::from_string("Ada");

auto result = client.put(key, value);
```

### Binary values

`Value` is binary-safe.

```cpp
Value value = Value::from_bytes({
    static_cast<unsigned char>(0x01),
    static_cast<unsigned char>(0x02),
    static_cast<unsigned char>(0x03),
});

client.put(Key{"binary/value"}, value);

// Read it back
auto result = client.get("binary/value");

if (result.is_ok())
{
    const auto &bytes = result.value().bytes();
    std::cout << "size: " << bytes.size() << "\n";
}
```

## Reading values

`get()` returns `Result<Value, Error>`. The caller must handle both success and error.

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

### Missing keys

A missing key returns an error result. Missing keys normally map to `not_found`.

```cpp
auto result = client.get("missing/key");

if (result.is_err())
{
    std::cout << result.error().code_string()
              << ": "
              << result.error().message()
              << "\n";
}
```

## Checking if a key exists

```cpp
if (client.contains("profile/name"))
{
    std::cout << "profile exists\n";
}

// With a Key object
Key key{"profile/name"};

if (client.contains(key))
{
    std::cout << "profile exists\n";
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

> A remove operation should be idempotent when possible. For the current SDK, reading after remove must return an error.

## Store size

```cpp
std::cout << client.size() << "\n";

if (client.empty())
{
    std::cout << "store is empty\n";
}
```

## Error handling

| Return type | Used by |
|-------------|---------|
| `Result<void, Error>` | `open()`, `put()`, `remove()`, `start_transport()`, `start_discovery()` |
| `Result<Value, Error>` | `get()` |

```cpp
auto result = client.put("key", "value");

if (result.is_err())
{
    const auto &error = result.error();

    std::cerr << error.code_string()
              << ": "
              << error.message()
              << "\n";
}
```

### Invalid keys

Empty keys are rejected before reaching the internal store engine.

```cpp
auto result = client.put("", "value");

if (result.is_err())
{
    std::cerr << result.error().message() << "\n";
}
```

### Operations while closed

Store operations fail when the client is closed.

```cpp
Client client;

auto result = client.put("key", "value");

if (result.is_err())
{
    std::cout << "client is not open\n";
}
```

## Local store and sync

A local `put()` is not only a key-value write. It can also create a sync operation internally. That means:

```cpp
client.put("profile/name", "Ada");
```

Can update:

- local materialized state
- WAL, if enabled
- sync outbox
- sync queue, if auto queue is enabled

### Inspecting sync state after local writes

```cpp
auto state = client.sync_state();

if (state.is_ok())
{
    std::cout << "outbox: " << state.value().outbox_size  << "\n";
    std::cout << "queued: " << state.value().queued_count << "\n";
}
```

## Recommended local store flow

```cpp
ClientOptions options = ClientOptions::persistent("node-1", "data/app.wal");

Client client{options};

auto open_result = client.open();

if (open_result.is_err())
{
    return 1;
}

auto write_result = client.put("key", "value");

if (write_result.is_err())
{
    return 1;
}

auto read_result = client.get("key");

if (read_result.is_ok())
{
    // use read_result.value()
}

client.close();
```

## Example: local profile storage

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    ClientOptions options = ClientOptions::memory_only("profile-node");

    Client client{options};

    if (client.open().is_err())
    {
        return 1;
    }

    client.put("profile/name",     "Ada");
    client.put("profile/language", "C++");
    client.put("profile/runtime",  "Softadastra");

    auto name     = client.get("profile/name");
    auto language = client.get("profile/language");
    auto runtime  = client.get("profile/runtime");

    if (name.is_ok())
    {
        std::cout << "name: "     << name.value().to_string()     << "\n";
    }
    if (language.is_ok())
    {
        std::cout << "language: " << language.value().to_string() << "\n";
    }
    if (runtime.is_ok())
    {
        std::cout << "runtime: "  << runtime.value().to_string()  << "\n";
    }

    std::cout << "entries: " << client.size() << "\n";

    client.close();

    return 0;
}
```

## Example: remove local session

```cpp
#include <iostream>
#include <softadastra/sdk.hpp>

int main()
{
    using namespace softadastra::sdk;

    Client client{ClientOptions::memory_only("session-node")};

    if (client.open().is_err())
    {
        return 1;
    }

    client.put("session/token", "temporary-token");

    std::cout << "before remove: "
              << (client.contains("session/token") ? "yes" : "no")
              << "\n";

    client.remove("session/token");

    std::cout << "after remove: "
              << (client.contains("session/token") ? "yes" : "no")
              << "\n";

    client.close();

    return 0;
}
```

## Design rule

The SDK local store API must stay small. The user should not need to manually touch:

- `softadastra::store::engine::StoreEngine`
- `softadastra::store::core::Operation`
- `softadastra::store::types::Key` / `Value`
- `softadastra::wal::writer::WalWriter`
- `softadastra::sync::engine::SyncEngine`

The SDK should expose only: `Client`, `ClientOptions`, `Key`, `Value`, `Result`, `Error`.

## Summary

The local store layer gives the SDK its first useful public behavior:

```cpp
client.open();
client.put("key", "value");
client.get("key");
client.remove("key");
client.close();
```

It is simple at the API level, but backed by the deeper Softadastra runtime:

```
SDK Client
  →  StoreEngine
  →  WAL
  →  Sync outbox
  →  Transport later
```

> A clean public API over the powerful Softadastra foundation.

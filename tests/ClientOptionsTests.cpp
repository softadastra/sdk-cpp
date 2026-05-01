/*
 * ClientOptionsTests.cpp
 */

#include <cassert>
#include <string>

#include <softadastra/sdk/ClientOptions.hpp>

namespace
{
  using softadastra::sdk::ClientOptions;

  void test_default_options_are_valid()
  {
    ClientOptions options;

    assert(options.is_valid());
    assert(options.valid());

    assert(options.node_id == "node-1");
    assert(options.version == "0.1.0");

    assert(options.enable_wal);
    assert(options.enable_transport);
    assert(options.enable_discovery);
  }

  void test_local_options()
  {
    auto options = ClientOptions::local("node-local");

    assert(options.is_valid());
    assert(options.node_id == "node-local");
    assert(options.display_name == "node-local");

    assert(options.transport_host == "127.0.0.1");
    assert(options.discovery_host == "127.0.0.1");
  }

  void test_memory_only_options()
  {
    auto options = ClientOptions::memory_only("node-memory");

    assert(options.is_valid());
    assert(options.node_id == "node-memory");

    assert(!options.enable_wal);
    assert(!options.enable_transport);
    assert(!options.enable_discovery);
  }

  void test_persistent_options()
  {
    auto options =
        ClientOptions::persistent(
            "node-persistent",
            "data/test-sdk.wal");

    assert(options.is_valid());
    assert(options.node_id == "node-persistent");

    assert(options.enable_wal);
    assert(options.wal_path == "data/test-sdk.wal");
    assert(options.auto_flush);
  }

  void test_invalid_node_id()
  {
    auto options = ClientOptions::local("");

    assert(!options.is_valid());
    assert(!options.valid());
  }

  void test_invalid_wal_path_when_wal_enabled()
  {
    auto options = ClientOptions::local("node-invalid-wal");

    options.enable_wal = true;
    options.wal_path.clear();

    assert(!options.is_valid());
  }

  void test_invalid_transport_port_when_transport_enabled()
  {
    auto options = ClientOptions::local("node-invalid-transport");

    options.enable_transport = true;
    options.transport_port = 0;

    assert(!options.is_valid());
  }

  void test_invalid_discovery_port_when_discovery_enabled()
  {
    auto options = ClientOptions::local("node-invalid-discovery");

    options.enable_discovery = true;
    options.discovery_port = 0;

    assert(!options.is_valid());
  }

  void test_store_config_conversion()
  {
    auto options =
        ClientOptions::persistent(
            "node-store-config",
            "data/store-config.wal");

    auto config = options.to_store_config();

    assert(config.enable_wal);
    assert(config.wal_path == "data/store-config.wal");
    assert(config.auto_flush == options.auto_flush);
    assert(config.is_valid());
  }

  void test_sync_config_conversion()
  {
    auto options = ClientOptions::local("node-sync-config");

    auto config = options.to_sync_config();

    assert(config.node_id == "node-sync-config");
    assert(config.batch_size == options.batch_size);
    assert(config.max_retries == options.max_retries);
    assert(config.is_valid());
  }

  void test_transport_config_conversion()
  {
    auto options = ClientOptions::local("node-transport-config");

    options.transport_host = "127.0.0.1";
    options.transport_port = 4040;

    auto config = options.to_transport_config();

    assert(config.bind_host == "127.0.0.1");
    assert(config.bind_port == 4040);
    assert(config.is_valid());
  }

  void test_metadata_options_conversion()
  {
    auto options = ClientOptions::local("node-metadata-config");

    options.display_name = "SDK Node";
    options.version = "0.2.0";

    auto metadata = options.to_metadata_options();

    assert(metadata.node_id == "node-metadata-config");
    assert(metadata.display_name == "SDK Node");
    assert(metadata.version == "0.2.0");
    assert(metadata.is_valid());
  }
}

int main()
{
  test_default_options_are_valid();
  test_local_options();
  test_memory_only_options();
  test_persistent_options();
  test_invalid_node_id();
  test_invalid_wal_path_when_wal_enabled();
  test_invalid_transport_port_when_transport_enabled();
  test_invalid_discovery_port_when_discovery_enabled();
  test_store_config_conversion();
  test_sync_config_conversion();
  test_transport_config_conversion();
  test_metadata_options_conversion();

  return 0;
}

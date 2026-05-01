/*
 * ClientTests.cpp
 */

#include <cassert>
#include <string>

#include <softadastra/sdk/Client.hpp>
#include <softadastra/sdk/ClientOptions.hpp>
#include <softadastra/sdk/Key.hpp>
#include <softadastra/sdk/Value.hpp>

namespace
{
  using softadastra::sdk::Client;
  using softadastra::sdk::ClientOptions;
  using softadastra::sdk::Key;
  using softadastra::sdk::Value;

  void test_client_starts_closed()
  {
    Client client;

    assert(!client.is_open());
    assert(!client.opened());
    assert(client.size() == 0);
    assert(client.empty());
  }

  void test_open_and_close_memory_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-open-close");

    Client client{options};

    auto open_result = client.open();

    assert(open_result.is_ok());
    assert(client.is_open());
    assert(client.opened());

    client.close();

    assert(!client.is_open());
    assert(!client.opened());
  }

  void test_put_and_get_string_value()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-put-get");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto put_result =
        client.put("app/name", "Softadastra SDK");

    assert(put_result.is_ok());
    assert(client.contains("app/name"));
    assert(client.size() == 1);
    assert(!client.empty());

    auto value_result = client.get("app/name");

    assert(value_result.is_ok());
    assert(value_result.value().to_string() == "Softadastra SDK");

    client.close();
  }

  void test_put_and_get_binary_value()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-binary-value");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    Value value =
        Value::from_bytes({
            static_cast<unsigned char>(0x01),
            static_cast<unsigned char>(0x02),
            static_cast<unsigned char>(0x03),
        });

    auto put_result =
        client.put(Key{"binary/value"}, value);

    assert(put_result.is_ok());

    auto value_result =
        client.get(Key{"binary/value"});

    assert(value_result.is_ok());
    assert(value_result.value().bytes().size() == 3);
    assert(value_result.value().bytes()[0] == 0x01);
    assert(value_result.value().bytes()[1] == 0x02);
    assert(value_result.value().bytes()[2] == 0x03);

    client.close();
  }

  void test_remove_value()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-remove");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto put_result =
        client.put("cache/session", "temporary");

    assert(put_result.is_ok());
    assert(client.contains("cache/session"));

    auto remove_result =
        client.remove("cache/session");

    assert(remove_result.is_ok());
    assert(!client.contains("cache/session"));

    auto value_result =
        client.get("cache/session");

    assert(value_result.is_err());

    client.close();
  }

  void test_get_missing_key_returns_error()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-missing-key");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto value_result =
        client.get("missing/key");

    assert(value_result.is_err());
    assert(value_result.error().has_error());

    client.close();
  }

  void test_operations_fail_when_client_is_closed()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-closed");

    Client client{options};

    auto put_result =
        client.put("key", "value");

    assert(put_result.is_err());

    auto get_result =
        client.get("key");

    assert(get_result.is_err());

    auto remove_result =
        client.remove("key");

    assert(remove_result.is_err());
  }

  void test_invalid_key_is_rejected()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-invalid-key");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto put_result =
        client.put(Key{}, Value::from_string("value"));

    assert(put_result.is_err());

    auto get_result =
        client.get(Key{});

    assert(get_result.is_err());

    auto remove_result =
        client.remove(Key{});

    assert(remove_result.is_err());

    client.close();
  }

  void test_tick_on_memory_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-tick");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto put_result =
        client.put("sync/key", "sync-value");

    assert(put_result.is_ok());

    auto tick_result =
        client.tick();

    assert(tick_result.is_ok());
    assert(tick_result.value().is_valid());

    client.close();
  }

  void test_sync_state_on_memory_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-sync-state");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto state_result =
        client.sync_state();

    assert(state_result.is_ok());
    assert(state_result.value().is_valid());

    client.close();
  }

  void test_transport_is_disabled_in_memory_only_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-transport-disabled");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto transport_result =
        client.start_transport();

    assert(transport_result.is_err());
    assert(!client.transport_running());

    client.close();
  }

  void test_discovery_is_disabled_in_memory_only_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-discovery-disabled");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto discovery_result =
        client.start_discovery();

    assert(discovery_result.is_err());
    assert(!client.discovery_running());

    auto peers_result =
        client.peers();

    assert(peers_result.is_err());

    client.close();
  }

  void test_node_info_is_available()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-node-info");

    options.display_name = "Test SDK Node";
    options.version = "0.1.0";

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto node_result =
        client.refresh_node_info();

    assert(node_result.is_ok());
    assert(node_result.value().node_id == "test-client-node-info");
    assert(node_result.value().display_name == "Test SDK Node");
    assert(node_result.value().version == "0.1.0");

    client.close();
  }

  void test_move_construct_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-move-construct");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto put_result =
        client.put("move/key", "move-value");

    assert(put_result.is_ok());

    Client moved{std::move(client)};

    assert(moved.is_open());
    assert(moved.contains("move/key"));

    auto value_result =
        moved.get("move/key");

    assert(value_result.is_ok());
    assert(value_result.value().to_string() == "move-value");

    moved.close();
  }

  void test_move_assign_client()
  {
    ClientOptions options =
        ClientOptions::memory_only("test-client-move-assign");

    Client client{options};

    auto open_result = client.open();
    assert(open_result.is_ok());

    auto put_result =
        client.put("assign/key", "assign-value");

    assert(put_result.is_ok());

    Client moved;

    moved = std::move(client);

    assert(moved.is_open());
    assert(moved.contains("assign/key"));

    auto value_result =
        moved.get("assign/key");

    assert(value_result.is_ok());
    assert(value_result.value().to_string() == "assign-value");

    moved.close();
  }
}

int main()
{
  test_client_starts_closed();
  test_open_and_close_memory_client();
  test_put_and_get_string_value();
  test_put_and_get_binary_value();
  test_remove_value();
  test_get_missing_key_returns_error();
  test_operations_fail_when_client_is_closed();
  test_invalid_key_is_rejected();
  test_tick_on_memory_client();
  test_sync_state_on_memory_client();
  test_transport_is_disabled_in_memory_only_client();
  test_discovery_is_disabled_in_memory_only_client();
  test_node_info_is_available();
  test_move_construct_client();
  test_move_assign_client();

  return 0;
}

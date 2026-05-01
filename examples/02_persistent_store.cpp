/*
 * 02_persistent_store.cpp
 *
 * WAL-backed local store example.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-persistent");

  options.enable_transport = false;
  options.enable_discovery = false;
  options.enable_wal = true;
  options.wal_path = "data/sdk-persistent-store.wal";
  options.auto_flush = true;

  Client client{options};

  auto open_result = client.open();

  if (open_result.is_err())
  {
    std::cerr << "failed to open client: "
              << open_result.error().message()
              << "\n";

    return 1;
  }

  auto put_result = client.put(
      "settings/theme",
      "dark");

  if (put_result.is_err())
  {
    std::cerr << "failed to store value: "
              << put_result.error().message()
              << "\n";

    return 1;
  }

  auto value_result = client.get("settings/theme");

  if (value_result.is_err())
  {
    std::cerr << "failed to read value: "
              << value_result.error().message()
              << "\n";

    return 1;
  }

  auto sync_result = client.sync_state();

  if (sync_result.is_err())
  {
    std::cerr << "failed to read sync state: "
              << sync_result.error().message()
              << "\n";

    return 1;
  }

  std::cout << "key          : settings/theme\n";
  std::cout << "value        : "
            << value_result.value().to_string()
            << "\n";

  std::cout << "wal path     : "
            << options.wal_path
            << "\n";

  std::cout << "store size   : "
            << client.size()
            << "\n";

  std::cout << "outbox size  : "
            << sync_result.value().outbox_size
            << "\n";

  client.close();

  return 0;
}

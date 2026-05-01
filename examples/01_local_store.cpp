/*
 * 01_local_store.cpp
 *
 * Minimal local-only Softadastra SDK example.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-local");

  options.enable_transport = false;
  options.enable_discovery = false;
  options.enable_wal = false;

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
      "app/name",
      "Softadastra SDK");

  if (put_result.is_err())
  {
    std::cerr << "failed to store value: "
              << put_result.error().message()
              << "\n";

    return 1;
  }

  auto value_result = client.get("app/name");

  if (value_result.is_err())
  {
    std::cerr << "failed to read value: "
              << value_result.error().message()
              << "\n";

    return 1;
  }

  std::cout << "key   : app/name\n";
  std::cout << "value : "
            << value_result.value().to_string()
            << "\n";

  std::cout << "size  : "
            << client.size()
            << "\n";

  client.close();

  return 0;
}

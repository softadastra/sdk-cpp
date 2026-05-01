/*
 * 03_remove_value.cpp
 *
 * Local remove/delete example.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-remove");

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
      "cache/session",
      "temporary-data");

  if (put_result.is_err())
  {
    std::cerr << "failed to store value: "
              << put_result.error().message()
              << "\n";

    return 1;
  }

  std::cout << "before remove\n";
  std::cout << "  contains : "
            << (client.contains("cache/session") ? "yes" : "no")
            << "\n";

  auto remove_result = client.remove("cache/session");

  if (remove_result.is_err())
  {
    std::cerr << "failed to remove value: "
              << remove_result.error().message()
              << "\n";

    return 1;
  }

  std::cout << "after remove\n";
  std::cout << "  contains : "
            << (client.contains("cache/session") ? "yes" : "no")
            << "\n";

  auto value_result = client.get("cache/session");

  if (value_result.is_err())
  {
    std::cout << "read result: "
              << value_result.error().code_string()
              << "\n";
  }

  client.close();

  return 0;
}

/*
 * 07_node_metadata.cpp
 *
 * Local node metadata example.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-metadata");

  options.display_name = "Softadastra SDK Node";
  options.version = "0.1.0";

  options.enable_wal = false;
  options.enable_transport = false;
  options.enable_discovery = false;

  Client client{options};

  auto open_result = client.open();

  if (open_result.is_err())
  {
    std::cerr << "failed to open client: "
              << open_result.error().message()
              << "\n";

    return 1;
  }

  auto node_result = client.refresh_node_info();

  if (node_result.is_err())
  {
    std::cerr << "failed to read node metadata: "
              << node_result.error().message()
              << "\n";

    client.close();
    return 1;
  }

  const auto &node = node_result.value();

  std::cout << "node metadata\n";
  std::cout << "  node id      : "
            << node.node_id
            << "\n";

  std::cout << "  display name : "
            << node.display_name
            << "\n";

  std::cout << "  hostname     : "
            << node.hostname
            << "\n";

  std::cout << "  os           : "
            << node.os_name
            << "\n";

  std::cout << "  version      : "
            << node.version
            << "\n";

  std::cout << "  uptime ms    : "
            << node.uptime_ms
            << "\n";

  std::cout << "  capabilities : ";

  if (node.capabilities.empty())
  {
    std::cout << "none";
  }
  else
  {
    for (std::size_t i = 0; i < node.capabilities.size(); ++i)
    {
      std::cout << node.capabilities[i];

      if (i + 1 < node.capabilities.size())
      {
        std::cout << ", ";
      }
    }
  }

  std::cout << "\n";

  client.close();

  return 0;
}

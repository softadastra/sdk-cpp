/*
 * 06_discovery.cpp
 *
 * Local peer discovery example.
 *
 * This example starts the SDK discovery service and prints the peers
 * currently known by the discovery layer.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-discovery-a");

  options.enable_wal = true;
  options.wal_path = "data/sdk-discovery.wal";
  options.auto_flush = true;

  options.enable_transport = true;
  options.transport_host = "127.0.0.1";
  options.transport_port = 4051;

  options.enable_discovery = true;
  options.discovery_host = "127.0.0.1";
  options.discovery_port = 5051;
  options.discovery_broadcast_host = "127.0.0.1";
  options.discovery_broadcast_port = 5052;

  Client client{options};

  auto open_result = client.open();

  if (open_result.is_err())
  {
    std::cerr << "failed to open client: "
              << open_result.error().message()
              << "\n";

    return 1;
  }

  auto discovery_result = client.start_discovery();

  if (discovery_result.is_err())
  {
    std::cerr << "failed to start discovery: "
              << discovery_result.error().message()
              << "\n";

    client.close();
    return 1;
  }

  auto peers_result = client.peers();

  if (peers_result.is_err())
  {
    std::cerr << "failed to read discovered peers: "
              << peers_result.error().message()
              << "\n";

    client.close();
    return 1;
  }

  std::cout << "discovery\n";
  std::cout << "  running : "
            << (client.discovery_running() ? "yes" : "no")
            << "\n";

  std::cout << "  bind    : "
            << options.discovery_host
            << ":"
            << options.discovery_port
            << "\n";

  std::cout << "  target  : "
            << options.discovery_broadcast_host
            << ":"
            << options.discovery_broadcast_port
            << "\n";

  std::cout << "\npeers\n";

  if (peers_result.value().empty())
  {
    std::cout << "  no peer discovered yet\n";
  }
  else
  {
    for (const auto &peer : peers_result.value())
    {
      std::cout << "  "
                << peer.node_id
                << " "
                << peer.host
                << ":"
                << peer.port
                << "\n";
    }
  }

  client.close();

  return 0;
}

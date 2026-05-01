/*
 * 05_tcp_peer_sync.cpp
 *
 * TCP peer connection example.
 *
 * This example shows how the SDK can be configured with transport enabled.
 * It does not require another node to be running. If no peer is available,
 * the connect call will fail cleanly and print the SDK error.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-tcp-a");

  options.enable_wal = true;
  options.wal_path = "data/sdk-tcp-peer-sync.wal";
  options.auto_flush = true;

  options.enable_transport = true;
  options.transport_host = "127.0.0.1";
  options.transport_port = 4041;

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

  auto transport_result = client.start_transport();

  if (transport_result.is_err())
  {
    std::cerr << "failed to start transport: "
              << transport_result.error().message()
              << "\n";

    client.close();
    return 1;
  }

  Peer peer{
      "node-tcp-b",
      "127.0.0.1",
      4042};

  auto connect_result = client.connect(peer);

  if (connect_result.is_err())
  {
    std::cout << "peer connection failed\n";
    std::cout << "  peer    : "
              << peer.node_id
              << "\n";
    std::cout << "  address : "
              << peer.host
              << ":"
              << peer.port
              << "\n";
    std::cout << "  error   : "
              << connect_result.error().message()
              << "\n";
  }
  else
  {
    std::cout << "connected to peer\n";
    std::cout << "  peer    : "
              << peer.node_id
              << "\n";
    std::cout << "  address : "
              << peer.host
              << ":"
              << peer.port
              << "\n";
  }

  auto put_result = client.put(
      "sync/message",
      "hello from node-tcp-a");

  if (put_result.is_err())
  {
    std::cerr << "failed to submit sync value: "
              << put_result.error().message()
              << "\n";

    client.close();
    return 1;
  }

  auto tick_result = client.tick();

  if (tick_result.is_err())
  {
    std::cerr << "failed to tick sync pipeline: "
              << tick_result.error().message()
              << "\n";

    client.close();
    return 1;
  }

  std::cout << "\nsync tick\n";
  std::cout << "  retried : "
            << tick_result.value().retried_count
            << "\n";
  std::cout << "  pruned  : "
            << tick_result.value().pruned_count
            << "\n";
  std::cout << "  batch   : "
            << tick_result.value().batch_size
            << "\n";

  std::cout << "\ntransport\n";
  std::cout << "  running : "
            << (client.transport_running() ? "yes" : "no")
            << "\n";

  client.close();

  return 0;
}

/*
 * 04_basic_sync.cpp
 *
 * Manual sync tick example.
 */

#include <iostream>

#include <softadastra/sdk.hpp>

int main()
{
  using namespace softadastra::sdk;

  ClientOptions options =
      ClientOptions::local("node-basic-sync");

  options.enable_transport = false;
  options.enable_discovery = false;
  options.enable_wal = true;
  options.wal_path = "data/sdk-basic-sync.wal";
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
      "profile/name",
      "Softadastra");

  if (put_result.is_err())
  {
    std::cerr << "failed to submit local value: "
              << put_result.error().message()
              << "\n";

    return 1;
  }

  auto before = client.sync_state();

  if (before.is_err())
  {
    std::cerr << "failed to read sync state: "
              << before.error().message()
              << "\n";

    return 1;
  }

  std::cout << "before tick\n";
  std::cout << "  outbox : "
            << before.value().outbox_size
            << "\n";
  std::cout << "  queued : "
            << before.value().queued_count
            << "\n";
  std::cout << "  failed : "
            << before.value().failed_count
            << "\n";

  auto tick_result = client.tick();

  if (tick_result.is_err())
  {
    std::cerr << "failed to tick sync pipeline: "
              << tick_result.error().message()
              << "\n";

    return 1;
  }

  std::cout << "\ntick result\n";
  std::cout << "  retried : "
            << tick_result.value().retried_count
            << "\n";
  std::cout << "  pruned  : "
            << tick_result.value().pruned_count
            << "\n";
  std::cout << "  batch   : "
            << tick_result.value().batch_size
            << "\n";

  auto after = client.sync_state();

  if (after.is_err())
  {
    std::cerr << "failed to read sync state after tick: "
              << after.error().message()
              << "\n";

    return 1;
  }

  std::cout << "\nafter tick\n";
  std::cout << "  outbox : "
            << after.value().outbox_size
            << "\n";
  std::cout << "  queued : "
            << after.value().queued_count
            << "\n";
  std::cout << "  failed : "
            << after.value().failed_count
            << "\n";

  client.close();

  return 0;
}

/**
 *
 *  @file TickResult.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/sdk-cpp
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra C++ SDK
 *
 */

#ifndef SOFTADASTRA_SDK_TICK_RESULT_HPP
#define SOFTADASTRA_SDK_TICK_RESULT_HPP

#include <cstddef>

#include <softadastra/sync/scheduler/SyncScheduler.hpp>

namespace softadastra::sdk
{
  namespace sync_scheduler = softadastra::sync::scheduler;

  /**
   * @brief Public SDK result of one synchronization tick.
   *
   * TickResult describes what happened when the SDK manually advanced the
   * sync pipeline once.
   *
   * It is intentionally smaller than the internal SyncScheduler::TickResult:
   * - it exposes counters useful to SDK users
   * - it hides internal SyncEnvelope batches
   * - transport remains behind the SDK client facade
   */
  struct TickResult
  {
    /**
     * @brief Number of expired operations requeued for retry.
     */
    std::size_t retried_count{0};

    /**
     * @brief Number of completed entries removed from the outbox.
     */
    std::size_t pruned_count{0};

    /**
     * @brief Number of sync envelopes produced for sending.
     */
    std::size_t batch_size{0};

    /**
     * @brief Creates an empty tick result.
     */
    TickResult() = default;

    /**
     * @brief Creates a tick result from explicit counters.
     *
     * @param retried Number of retried operations.
     * @param pruned Number of pruned entries.
     * @param batch Number of produced outbound envelopes.
     */
    TickResult(
        std::size_t retried,
        std::size_t pruned,
        std::size_t batch)
        : retried_count(retried),
          pruned_count(pruned),
          batch_size(batch)
    {
    }

    /**
     * @brief Creates an SDK tick result from internal scheduler output.
     *
     * @param result Internal scheduler tick result.
     * @return SDK tick result.
     */
    [[nodiscard]] static TickResult from_scheduler(
        const sync_scheduler::SyncScheduler::TickResult &result) noexcept
    {
      return TickResult{
          result.retried_count,
          result.pruned_count,
          result.batch_size()};
    }

    /**
     * @brief Returns true if the tick produced any work.
     */
    [[nodiscard]] bool has_work() const noexcept
    {
      return retried_count > 0 ||
             pruned_count > 0 ||
             batch_size > 0;
    }

    /**
     * @brief Returns true if retry work happened.
     */
    [[nodiscard]] bool retried() const noexcept
    {
      return retried_count > 0;
    }

    /**
     * @brief Returns true if pruning happened.
     */
    [[nodiscard]] bool pruned() const noexcept
    {
      return pruned_count > 0;
    }

    /**
     * @brief Returns true if outbound sync work was produced.
     */
    [[nodiscard]] bool produced_batch() const noexcept
    {
      return batch_size > 0;
    }

    /**
     * @brief Clears all counters.
     */
    void clear() noexcept
    {
      retried_count = 0;
      pruned_count = 0;
      batch_size = 0;
    }
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_TICK_RESULT_HPP

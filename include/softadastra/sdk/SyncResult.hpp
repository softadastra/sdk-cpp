/**
 *
 *  @file SyncResult.hpp
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

#ifndef SOFTADASTRA_SDK_SYNC_RESULT_HPP
#define SOFTADASTRA_SDK_SYNC_RESULT_HPP

#include <cstddef>
#include <cstdint>

#include <softadastra/sync/core/SyncState.hpp>

namespace softadastra::sdk
{
  namespace sync_core = softadastra::sync::core;

  /**
   * @brief Public SDK synchronization result.
   *
   * SyncResult exposes a compact view of the current synchronization state.
   *
   * It is returned by SDK methods that submit, flush, retry, or inspect sync
   * work without exposing the internal SyncEngine types directly.
   */
  struct SyncResult
  {
    /**
     * @brief Number of operations stored in the outbox.
     */
    std::size_t outbox_size{0};

    /**
     * @brief Number of operations waiting to be sent.
     */
    std::size_t queued_count{0};

    /**
     * @brief Number of operations currently in flight.
     */
    std::size_t in_flight_count{0};

    /**
     * @brief Number of acknowledged operations.
     */
    std::size_t acknowledged_count{0};

    /**
     * @brief Number of failed operations.
     */
    std::size_t failed_count{0};

    /**
     * @brief Last local version submitted to sync.
     */
    std::uint64_t last_submitted_version{0};

    /**
     * @brief Last remote version applied locally.
     */
    std::uint64_t last_applied_remote_version{0};

    /**
     * @brief Total retry attempts performed by the sync engine.
     */
    std::uint64_t total_retries{0};

    /**
     * @brief Creates an empty sync result.
     */
    SyncResult() = default;

    /**
     * @brief Creates a sync result from explicit counters.
     *
     * @param outbox Number of outbox entries.
     * @param queued Number of queued operations.
     * @param in_flight Number of in-flight operations.
     * @param acknowledged Number of acknowledged operations.
     * @param failed Number of failed operations.
     */
    SyncResult(
        std::size_t outbox,
        std::size_t queued,
        std::size_t in_flight,
        std::size_t acknowledged,
        std::size_t failed)
        : outbox_size(outbox),
          queued_count(queued),
          in_flight_count(in_flight),
          acknowledged_count(acknowledged),
          failed_count(failed)
    {
    }

    /**
     * @brief Creates an SDK sync result from internal SyncState.
     *
     * @param state Internal sync state.
     * @return SDK sync result.
     */
    [[nodiscard]] static SyncResult from_state(
        const sync_core::SyncState &state) noexcept
    {
      SyncResult result;

      result.outbox_size = state.outbox_size;
      result.queued_count = state.queued_count;
      result.in_flight_count = state.in_flight_count;
      result.acknowledged_count = state.acknowledged_count;
      result.failed_count = state.failed_count;
      result.last_submitted_version = state.last_submitted_version;
      result.last_applied_remote_version = state.last_applied_remote_version;
      result.total_retries = state.total_retries;

      return result;
    }

    /**
     * @brief Converts this SDK result to an internal SyncState.
     *
     * @return Internal sync state.
     */
    [[nodiscard]] sync_core::SyncState to_state() const noexcept
    {
      sync_core::SyncState state;

      state.outbox_size = outbox_size;
      state.queued_count = queued_count;
      state.in_flight_count = in_flight_count;
      state.acknowledged_count = acknowledged_count;
      state.failed_count = failed_count;
      state.last_submitted_version = last_submitted_version;
      state.last_applied_remote_version = last_applied_remote_version;
      state.total_retries = total_retries;

      return state;
    }

    /**
     * @brief Returns true if there are queued operations.
     */
    [[nodiscard]] bool has_queued() const noexcept
    {
      return queued_count > 0;
    }

    /**
     * @brief Returns true if there are in-flight operations.
     */
    [[nodiscard]] bool has_in_flight() const noexcept
    {
      return in_flight_count > 0;
    }

    /**
     * @brief Returns true if there are failed operations.
     */
    [[nodiscard]] bool has_failed() const noexcept
    {
      return failed_count > 0;
    }

    /**
     * @brief Returns true if the sync pipeline has pending work.
     */
    [[nodiscard]] bool has_work() const noexcept
    {
      return queued_count > 0 ||
             in_flight_count > 0 ||
             failed_count > 0;
    }

    /**
     * @brief Returns true if no sync work is tracked.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return outbox_size == 0 &&
             queued_count == 0 &&
             in_flight_count == 0 &&
             acknowledged_count == 0 &&
             failed_count == 0 &&
             last_submitted_version == 0 &&
             last_applied_remote_version == 0 &&
             total_retries == 0;
    }

    /**
     * @brief Returns the number of tracked operation states.
     */
    [[nodiscard]] std::size_t total_tracked() const noexcept
    {
      return queued_count +
             in_flight_count +
             acknowledged_count +
             failed_count;
    }

    /**
     * @brief Clears all counters.
     */
    void clear() noexcept
    {
      outbox_size = 0;
      queued_count = 0;
      in_flight_count = 0;
      acknowledged_count = 0;
      failed_count = 0;
      last_submitted_version = 0;
      last_applied_remote_version = 0;
      total_retries = 0;
    }
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_SYNC_RESULT_HPP

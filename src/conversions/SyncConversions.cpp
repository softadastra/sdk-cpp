/*
 * SyncConversions.cpp
 */

#include <softadastra/sdk/SyncResult.hpp>
#include <softadastra/sdk/TickResult.hpp>

/*
 * Sync conversions are implemented inline in their SDK value objects:
 *
 * - SyncResult::from_state(...)
 * - SyncResult::to_state()
 * - TickResult::from_scheduler(...)
 *
 * Reason:
 * - these conversions are tiny
 * - they are part of the public SDK value-object API
 * - keeping them inline avoids unnecessary exported symbols
 *
 * This translation unit is intentionally kept so the conversion layer has a
 * stable file layout and can grow later when the SDK exposes richer sync
 * conversion helpers.
 */

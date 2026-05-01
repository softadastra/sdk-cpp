/*
 * DiscoveryConversions.cpp
 */

#include <softadastra/sdk/Peer.hpp>

/*
 * Discovery conversions are implemented inline in Peer.hpp:
 *
 * - Peer::from_discovery(...)
 *
 * Reason:
 * - Peer is the public SDK representation of a discovered node
 * - discovery::Peer already exposes the clean public discovery peer shape
 * - keeping the conversion close to Peer keeps the SDK API easy to read
 *
 * This translation unit is intentionally kept so the conversion layer has a
 * stable file layout and can grow later when the SDK exposes richer discovery
 * helpers.
 */

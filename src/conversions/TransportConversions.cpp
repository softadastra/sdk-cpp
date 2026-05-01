/*
 * TransportConversions.cpp
 */

#include <softadastra/sdk/Peer.hpp>

/*
 * Transport conversions are implemented inline in Peer.hpp:
 *
 * - Peer::from_transport(...)
 * - Peer::to_transport()
 *
 * Reason:
 * - Peer is a small SDK value object
 * - conversion to transport::core::PeerInfo is part of its public purpose
 * - keeping the conversion close to the type makes usage clearer
 *
 * This translation unit is intentionally kept so the conversion layer has a
 * stable file layout and can grow later when the SDK exposes richer transport
 * helpers.
 */

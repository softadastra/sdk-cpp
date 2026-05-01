/*
 * MetadataConversions.cpp
 */

#include <softadastra/sdk/NodeInfo.hpp>

/*
 * Metadata conversions are implemented inline in NodeInfo.hpp:
 *
 * - NodeInfo::from_metadata(...)
 * - NodeInfo::to_metadata()
 *
 * Reason:
 * - NodeInfo is the public SDK representation of local node metadata
 * - metadata::core::NodeMetadata is an internal Softadastra module type
 * - keeping the conversion close to NodeInfo makes the SDK API predictable
 *
 * This translation unit is intentionally kept so the conversion layer has a
 * stable file layout and can grow later when the SDK exposes richer metadata
 * conversion helpers.
 */

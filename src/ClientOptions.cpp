/*
 * ClientOptions.cpp
 */

#include <softadastra/sdk/ClientOptions.hpp>

/*
 * ClientOptions is intentionally implemented inline in the header.
 *
 * Reason:
 * - it is a small public configuration value object
 * - most methods are simple conversions to internal Softadastra configs
 * - keeping them inline makes the SDK easier to consume
 * - it avoids unnecessary exported symbols for configuration helpers
 *
 * This translation unit is kept so the build system has a stable source file
 * for the SDK module and can be extended later without changing the project
 * structure.
 */

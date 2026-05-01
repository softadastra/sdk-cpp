/**
 *
 *  @file sdk.hpp
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

#ifndef SOFTADASTRA_SDK_HPP
#define SOFTADASTRA_SDK_HPP

#include <softadastra/sdk/Result.hpp>
#include <softadastra/sdk/Error.hpp>
#include <softadastra/sdk/Key.hpp>
#include <softadastra/sdk/Value.hpp>
#include <softadastra/sdk/ClientOptions.hpp>
#include <softadastra/sdk/Peer.hpp>
#include <softadastra/sdk/NodeInfo.hpp>
#include <softadastra/sdk/SyncResult.hpp>
#include <softadastra/sdk/TickResult.hpp>
#include <softadastra/sdk/Client.hpp>

/**
 * @brief Main namespace for the Softadastra C++ SDK.
 *
 * The SDK is the public developer-facing API built on top of the
 * Softadastra runtime modules.
 *
 * It intentionally exposes a small surface:
 * - Client
 * - ClientOptions
 * - Key
 * - Value
 * - Result
 * - Error
 * - Peer
 * - NodeInfo
 * - SyncResult
 * - TickResult
 *
 * Internal modules such as WAL, Store, Sync, Transport, Discovery,
 * and Metadata remain hidden behind the SDK facade.
 */
namespace softadastra::sdk
{
}

#endif // SOFTADASTRA_SDK_HPP

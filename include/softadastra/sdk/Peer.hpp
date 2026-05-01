/**
 *
 *  @file Peer.hpp
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

#ifndef SOFTADASTRA_SDK_PEER_HPP
#define SOFTADASTRA_SDK_PEER_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/discovery/Peer.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>

namespace softadastra::sdk
{
  namespace discovery_api = softadastra::discovery;
  namespace transport_core = softadastra::transport::core;

  /**
   * @brief Public SDK peer representation.
   *
   * Peer describes a remote or local node that the SDK can discover,
   * connect to, or exchange sync data with.
   *
   * It intentionally exposes only the stable developer-facing fields:
   * - node id
   * - host
   * - port
   *
   * Internal discovery and transport objects remain hidden behind this type.
   */
  struct Peer
  {
    /**
     * @brief Logical peer node identifier.
     */
    std::string node_id{};

    /**
     * @brief Peer host or IP address.
     */
    std::string host{"127.0.0.1"};

    /**
     * @brief Peer transport port.
     */
    std::uint16_t port{0};

    /**
     * @brief Creates an empty invalid peer.
     */
    Peer() = default;

    /**
     * @brief Creates a peer.
     *
     * @param peer_node_id Logical peer node id.
     * @param peer_host Peer host or IP address.
     * @param peer_port Peer transport port.
     */
    Peer(
        std::string peer_node_id,
        std::string peer_host,
        std::uint16_t peer_port)
        : node_id(std::move(peer_node_id)),
          host(std::move(peer_host)),
          port(peer_port)
    {
    }

    /**
     * @brief Creates a localhost peer.
     *
     * @param peer_node_id Logical peer node id.
     * @param peer_port Peer transport port.
     * @return SDK peer.
     */
    [[nodiscard]] static Peer local(
        std::string peer_node_id,
        std::uint16_t peer_port)
    {
      return Peer{
          std::move(peer_node_id),
          "127.0.0.1",
          peer_port};
    }

    /**
     * @brief Creates a peer from Softadastra discovery peer.
     *
     * @param peer Discovery peer.
     * @return SDK peer.
     */
    [[nodiscard]] static Peer from_discovery(
        const discovery_api::Peer &peer)
    {
      return Peer{
          peer.node_id,
          peer.host,
          peer.port};
    }

    /**
     * @brief Creates a peer from Softadastra transport peer info.
     *
     * @param peer Transport peer info.
     * @return SDK peer.
     */
    [[nodiscard]] static Peer from_transport(
        const transport_core::PeerInfo &peer)
    {
      return Peer{
          peer.node_id,
          peer.host,
          peer.port};
    }

    /**
     * @brief Converts this SDK peer to Softadastra discovery peer.
     *
     * @return Discovery peer.
     */
    [[nodiscard]] discovery_api::Peer to_discovery() const
    {
      return discovery_api::Peer{
          node_id,
          host,
          port};
    }

    /**
     * @brief Converts this SDK peer to Softadastra transport peer info.
     *
     * @return Transport peer info.
     */
    [[nodiscard]] transport_core::PeerInfo to_transport() const
    {
      return transport_core::PeerInfo{
          node_id,
          host,
          port};
    }

    /**
     * @brief Returns true if this peer points to localhost.
     */
    [[nodiscard]] bool is_localhost() const noexcept
    {
      return host == "127.0.0.1" ||
             host == "localhost";
    }

    /**
     * @brief Returns true if the peer is usable.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !node_id.empty() &&
             !host.empty() &&
             port != 0;
    }

    /**
     * @brief Backward-compatible valid alias.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Clears the peer.
     */
    void clear() noexcept
    {
      node_id.clear();
      host.clear();
      port = 0;
    }

    /**
     * @brief Compares two peers for equality.
     */
    [[nodiscard]] friend bool operator==(
        const Peer &a,
        const Peer &b) noexcept
    {
      return a.node_id == b.node_id &&
             a.host == b.host &&
             a.port == b.port;
    }

    /**
     * @brief Compares two peers for inequality.
     */
    [[nodiscard]] friend bool operator!=(
        const Peer &a,
        const Peer &b) noexcept
    {
      return !(a == b);
    }
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_PEER_HPP

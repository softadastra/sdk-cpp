/**
 *
 *  @file NodeInfo.hpp
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

#ifndef SOFTADASTRA_SDK_NODE_INFO_HPP
#define SOFTADASTRA_SDK_NODE_INFO_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <softadastra/core/time/Duration.hpp>
#include <softadastra/core/time/Timestamp.hpp>
#include <softadastra/metadata/core/NodeMetadata.hpp>
#include <softadastra/metadata/types/CapabilityType.hpp>

namespace softadastra::sdk
{
  namespace core_time = softadastra::core::time;
  namespace metadata_core = softadastra::metadata::core;
  namespace metadata_types = softadastra::metadata::types;

  /**
   * @brief Public SDK node information.
   *
   * NodeInfo exposes a simplified view of a Softadastra node.
   *
   * It is built from the metadata module but hides internal metadata objects
   * from SDK users.
   */
  struct NodeInfo
  {
    /**
     * @brief Logical node identifier.
     */
    std::string node_id{};

    /**
     * @brief Human-readable node name.
     */
    std::string display_name{};

    /**
     * @brief Local hostname.
     */
    std::string hostname{};

    /**
     * @brief Operating system name.
     */
    std::string os_name{};

    /**
     * @brief Runtime or product version.
     */
    std::string version{};

    /**
     * @brief Node start timestamp.
     */
    core_time::Timestamp started_at{};

    /**
     * @brief Current uptime.
     */
    core_time::Duration uptime{};

    /**
     * @brief Declared node capabilities.
     */
    std::vector<metadata_types::CapabilityType> capabilities{};

    /**
     * @brief Creates an empty invalid node info object.
     */
    NodeInfo() = default;

    /**
     * @brief Creates node info from explicit fields.
     *
     * @param local_node_id Logical node id.
     * @param local_display_name Human-readable node name.
     * @param local_hostname Hostname.
     * @param local_os_name Operating system name.
     * @param local_version Runtime or product version.
     */
    NodeInfo(
        std::string local_node_id,
        std::string local_display_name,
        std::string local_hostname,
        std::string local_os_name,
        std::string local_version)
        : node_id(std::move(local_node_id)),
          display_name(std::move(local_display_name)),
          hostname(std::move(local_hostname)),
          os_name(std::move(local_os_name)),
          version(std::move(local_version)),
          started_at(core_time::Timestamp::now())
    {
      if (display_name.empty())
      {
        display_name = node_id;
      }
    }

    /**
     * @brief Creates SDK node info from metadata module node metadata.
     *
     * @param metadata Internal node metadata.
     * @return SDK node info.
     */
    [[nodiscard]] static NodeInfo from_metadata(
        const metadata_core::NodeMetadata &metadata)
    {
      NodeInfo info;

      info.node_id = metadata.identity.node_id;
      info.display_name = metadata.identity.display_name;
      info.hostname = metadata.runtime.hostname;
      info.os_name = metadata.runtime.os_name;
      info.version = metadata.runtime.version;
      info.started_at = metadata.runtime.started_at;
      info.uptime = metadata.runtime.uptime;
      info.capabilities = metadata.capabilities.values;

      if (info.display_name.empty())
      {
        info.display_name = info.node_id;
      }

      return info;
    }

    /**
     * @brief Converts SDK node info to metadata module node metadata.
     *
     * @return Internal node metadata.
     */
    [[nodiscard]] metadata_core::NodeMetadata to_metadata() const
    {
      metadata_core::NodeCapabilities node_capabilities;
      node_capabilities.values = capabilities;

      return metadata_core::NodeMetadata{
          metadata_core::NodeIdentity{
              node_id,
              display_name.empty() ? node_id : display_name},
          metadata_core::NodeRuntimeInfo{
              hostname,
              os_name,
              version},
          node_capabilities};
    }

    /**
     * @brief Returns the best display label.
     *
     * @return display_name when present, otherwise node_id.
     */
    [[nodiscard]] const std::string &label() const noexcept
    {
      return display_name.empty() ? node_id : display_name;
    }

    /**
     * @brief Returns true if a capability is present.
     *
     * @param capability Capability to search.
     * @return true when capability is declared.
     */
    [[nodiscard]] bool has_capability(
        metadata_types::CapabilityType capability) const noexcept
    {
      for (const auto value : capabilities)
      {
        if (value == capability)
        {
          return true;
        }
      }

      return false;
    }

    /**
     * @brief Returns true if foundation runtime capabilities are present.
     */
    [[nodiscard]] bool has_foundation_capabilities() const noexcept
    {
      for (const auto value : capabilities)
      {
        if (metadata_types::is_foundation(value))
        {
          return true;
        }
      }

      return false;
    }

    /**
     * @brief Returns uptime in milliseconds.
     */
    [[nodiscard]] core_time::Duration::rep uptime_ms() const noexcept
    {
      return uptime.millis();
    }

    /**
     * @brief Returns true if node info is usable.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !node_id.empty() &&
             !hostname.empty() &&
             !os_name.empty() &&
             !version.empty();
    }

    /**
     * @brief Backward-compatible valid alias.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Clears the node info.
     */
    void clear() noexcept
    {
      node_id.clear();
      display_name.clear();
      hostname.clear();
      os_name.clear();
      version.clear();
      started_at = core_time::Timestamp{};
      uptime = core_time::Duration{};
      capabilities.clear();
    }
  };

} // namespace softadastra::sdk

#endif // SOFTADASTRA_SDK_NODE_INFO_HPP

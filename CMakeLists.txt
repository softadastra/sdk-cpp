cmake_minimum_required(VERSION 3.20)

project(sdk-cpp
    VERSION 0.1.0
    DESCRIPTION "C++ SDK for building local-first applications with Softadastra"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

# ========================
# Options
# ========================
option(SDK_CPP_BUILD_EXAMPLES "Build sdk-cpp examples" ON)
option(SDK_CPP_BUILD_TESTS "Build sdk-cpp tests" OFF)
option(SDK_CPP_INSTALL "Install sdk-cpp package" ON)
option(SDK_CPP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# ========================
# Global settings
# ========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ========================
# Softadastra dependency
# ========================
find_package(softadastra CONFIG QUIET)

if(NOT softadastra_FOUND)
    set(SDK_CPP_SOFTADASTRA_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../softadastra"
        CACHE PATH "Path to Softadastra source tree"
    )

    if(EXISTS "${SDK_CPP_SOFTADASTRA_SOURCE_DIR}/CMakeLists.txt")
        message(STATUS "[sdk-cpp] using local Softadastra source: ${SDK_CPP_SOFTADASTRA_SOURCE_DIR}")

        set(SOFTADASTRA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SOFTADASTRA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(SOFTADASTRA_BUILD_APPS OFF CACHE BOOL "" FORCE)
        set(SOFTADASTRA_BUILD_CLI OFF CACHE BOOL "" FORCE)
        set(SOFTADASTRA_INSTALL OFF CACHE BOOL "" FORCE)

        add_subdirectory(
            "${SDK_CPP_SOFTADASTRA_SOURCE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/softadastra"
        )
    else()
        message(FATAL_ERROR
            "Softadastra was not found.\n"
            "Install Softadastra first or set SDK_CPP_SOFTADASTRA_SOURCE_DIR."
        )
    endif()
else()
    message(STATUS "[sdk-cpp] using installed Softadastra package")
endif()

# ========================
# Helper: resolve Softadastra targets
# ========================
function(sdk_cpp_find_softadastra_target output_var)
    foreach(candidate IN LISTS ARGN)
        if(TARGET "${candidate}")
            set(${output_var} "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    set(${output_var} "" PARENT_SCOPE)
endfunction()

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_CORE_TARGET
    softadastra::core
    softadastra_core
)

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_WAL_TARGET
    softadastra::wal
    softadastra_wal
)

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_STORE_TARGET
    softadastra::store
    softadastra_store
)

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_SYNC_TARGET
    softadastra::sync
    softadastra_sync
)

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_TRANSPORT_TARGET
    softadastra::transport
    softadastra_transport
)

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_DISCOVERY_TARGET
    softadastra::discovery
    softadastra_discovery
)

sdk_cpp_find_softadastra_target(SDK_CPP_SOFTADASTRA_METADATA_TARGET
    softadastra::metadata
    softadastra_metadata
)

set(SDK_CPP_REQUIRED_TARGETS
    SDK_CPP_SOFTADASTRA_CORE_TARGET
    SDK_CPP_SOFTADASTRA_WAL_TARGET
    SDK_CPP_SOFTADASTRA_STORE_TARGET
)

foreach(required_target_var IN LISTS SDK_CPP_REQUIRED_TARGETS)
    if("${${required_target_var}}" STREQUAL "")
        message(FATAL_ERROR
            "Required Softadastra target not found: ${required_target_var}"
        )
    endif()
endforeach()

# ========================
# SDK library
# ========================
add_library(sdk-cpp
    src/Error.cpp
    src/ClientOptions.cpp
    src/Client.cpp
    src/conversions/StoreConversions.cpp
    src/conversions/SyncConversions.cpp
    src/conversions/TransportConversions.cpp
    src/conversions/DiscoveryConversions.cpp
    src/conversions/MetadataConversions.cpp
)

add_library(sdk-cpp::sdk-cpp ALIAS sdk-cpp)

target_include_directories(sdk-cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(sdk-cpp
    PUBLIC
        ${SDK_CPP_SOFTADASTRA_CORE_TARGET}
        ${SDK_CPP_SOFTADASTRA_WAL_TARGET}
        ${SDK_CPP_SOFTADASTRA_STORE_TARGET}
)

if(NOT "${SDK_CPP_SOFTADASTRA_SYNC_TARGET}" STREQUAL "")
    target_link_libraries(sdk-cpp PUBLIC ${SDK_CPP_SOFTADASTRA_SYNC_TARGET})
endif()

if(NOT "${SDK_CPP_SOFTADASTRA_TRANSPORT_TARGET}" STREQUAL "")
    target_link_libraries(sdk-cpp PUBLIC ${SDK_CPP_SOFTADASTRA_TRANSPORT_TARGET})
endif()

if(NOT "${SDK_CPP_SOFTADASTRA_DISCOVERY_TARGET}" STREQUAL "")
    target_link_libraries(sdk-cpp PUBLIC ${SDK_CPP_SOFTADASTRA_DISCOVERY_TARGET})
endif()

if(NOT "${SDK_CPP_SOFTADASTRA_METADATA_TARGET}" STREQUAL "")
    target_link_libraries(sdk-cpp PUBLIC ${SDK_CPP_SOFTADASTRA_METADATA_TARGET})
endif()

target_compile_features(sdk-cpp PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(sdk-cpp PRIVATE /W4)
    if(SDK_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(sdk-cpp PRIVATE /WX)
    endif()
else()
    target_compile_options(sdk-cpp PRIVATE -Wall -Wextra -Wpedantic)
    if(SDK_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(sdk-cpp PRIVATE -Werror)
    endif()
endif()

# ========================
# Examples
# ========================
if(SDK_CPP_BUILD_EXAMPLES)
    function(sdk_cpp_add_example example_name source_file)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
            add_executable(${example_name} ${source_file})
            target_link_libraries(${example_name} PRIVATE sdk-cpp)
            target_compile_features(${example_name} PRIVATE cxx_std_20)
        endif()
    endfunction()

    sdk_cpp_add_example(sdk_cpp_01_local_store examples/01_local_store.cpp)
    sdk_cpp_add_example(sdk_cpp_02_persistent_store examples/02_persistent_store.cpp)
    sdk_cpp_add_example(sdk_cpp_03_remove_value examples/03_remove_value.cpp)
    sdk_cpp_add_example(sdk_cpp_04_basic_sync examples/04_basic_sync.cpp)
    sdk_cpp_add_example(sdk_cpp_05_tcp_peer_sync examples/05_tcp_peer_sync.cpp)
    sdk_cpp_add_example(sdk_cpp_06_discovery examples/06_discovery.cpp)
    sdk_cpp_add_example(sdk_cpp_07_node_metadata examples/07_node_metadata.cpp)
endif()

# ========================
# Tests
# ========================
if(SDK_CPP_BUILD_TESTS)
    enable_testing()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()

# ========================
# Install
# ========================
if(SDK_CPP_INSTALL)
    install(
        TARGETS sdk-cpp
        EXPORT sdk-cppTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT sdk-cppTargets
        FILE sdk-cppTargets.cmake
        NAMESPACE sdk-cpp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdk-cpp
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/sdk-cppConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdk-cpp-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/sdk-cppConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/sdk-cpp"
        PATH_VARS CMAKE_INSTALL_LIBDIR
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/sdk-cppConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/sdk-cppConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/sdk-cpp"
    )

    install(
        FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/sdk-cpp"
        OPTIONAL
    )
endif()

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "sdk-cpp configured (${PROJECT_VERSION})")
message(STATUS "Build examples: ${SDK_CPP_BUILD_EXAMPLES}")
message(STATUS "Build tests:    ${SDK_CPP_BUILD_TESTS}")
message(STATUS "Install:        ${SDK_CPP_INSTALL}")
message(STATUS "Warnings=error: ${SDK_CPP_WARNINGS_AS_ERRORS}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "------------------------------------------------------")

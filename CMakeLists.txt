cmake_minimum_required(VERSION 3.9)

if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
else()
  set(NOT_SUBPROJECT OFF)
endif()

project(SingleThreadSharedPtr LANGUAGES CXX)
set(SingleThreadSharedPtr_CMAKE_CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SingleThreadSharedPtr")

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(CheckIPOSupported)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(include)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Taken from catch2 project wisdom :)
# Only perform the installation steps when Catch is not being used as
# a subproject via `add_subdirectory`, or the destinations will break,
# see https://github.com/catchorg/Catch2/issues/1373
if (NOT_SUBPROJECT)
    set(SingleThreadSharedPtr_VERSION 0.0.1)
    set(SingleThreadSharedPtr_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake CACHE STRING "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
    set(SingleThreadSharedPtr_ConfigVersionFile ${CMAKE_CURRENT_BINARY_DIR}/cmake/SingleThreadSharedPtrConfigVersion.cmake)
    set(SingleThreadSharedPtr_ConfigFile ${CMAKE_CURRENT_BINARY_DIR}/cmake/SingleThreadSharedPtrConfig.cmake)
    set(ConfigPackageLocation lib/cmake/SingleThreadSharedPtr)

    # Generate the version, config and target files into the build directory.
    write_basic_package_version_file(
            ${SingleThreadSharedPtr_ConfigVersionFile}
        VERSION
            ${SingleThreadSharedPtr_VERSION}
        COMPATIBILITY
            AnyNewerVersion
    )

    # create an export configuration
    export(
        EXPORT
            SingleThreadSharedPtrTargets
        NAMESPACE
            SingleThreadSharedPtr::
        FILE
            "${CMAKE_CURRENT_BINARY_DIR}/cmake/SingleThreadSharedPtrTargets.cmake"
    )

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/SingleThreadSharedPtrConfig.cmake.in
        ${SingleThreadSharedPtr_ConfigFile}
        INSTALL_DESTINATION ${SingleThreadSharedPtr_CMAKE_DIR}
    )

    # Install version, config and target files.
    install(
        FILES
            ${project_config}
            ${version_config}
        DESTINATION
            ${SingleThreadSharedPtr_CMAKE_DIR}
    )

    install(
        EXPORT
            SingleThreadSharedPtrTargets
        NAMESPACE
            SingleThreadSharedPtr::
        FILE
            SingleThreadSharedPtrTargets.cmake
        DESTINATION
            ${ConfigPackageLocation}
    )

    install(
      FILES
        "${SingleThreadSharedPtr_ConfigFile}"
        "${SingleThreadSharedPtr_ConfigVersionFile}"
      DESTINATION
        ${ConfigPackageLocation}
    )
endif(NOT_SUBPROJECT)

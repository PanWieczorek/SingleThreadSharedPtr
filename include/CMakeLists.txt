set(LibName SingleThreadSharedPtr)

set(SingleThreadSharedPtr_INC
    single_thread_shared_ptr/single_thread_shared_ptr.hpp
)

add_library(${LibName} INTERFACE)
add_library(${LibName}::${LibName} ALIAS ${LibName})

# make ide happy
add_custom_target(${LibName}_ide SOURCES ${single_thread_shared_ptr_INC})

target_include_directories(${LibName}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (NOT_SUBPROJECT)
    install(
        DIRECTORY
            "${SOURCES_DIR}"
        DESTINATION
            "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    install(
        TARGETS
            ${LibName}
        EXPORT
            SingleThreadSharedPtrTargets
        INCLUDES DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
      EXPORT
        SingleThreadSharedPtrTargets
      NAMESPACE
        SingleThreadSharedPtr::
      DESTINATION
        ${SingleThreadSharedPtr_CMAKE_CONFIG_DESTINATION}
    )

endif(NOT_SUBPROJECT)

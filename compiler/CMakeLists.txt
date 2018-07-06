include(ExternalProject)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILER_CARGO_COMMAND cargo build)
else ()
    set(COMPILER_CARGO_COMMAND cargo build --release)
endif ()

set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/compiler)
ExternalProject_Add(
    compiler
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${COMPILER_CARGO_COMMAND}
    BINARY_DIR "${CMAKE_SOURCE_DIR}/compiler"
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
    USE_TERMINAL_BUILD ON
)

add_subdirectory(llvmmaxim)

set(CMAKE_AUTORCC ON)

if (AXIOM_STATIC_LINK)
    set(Qt5_USE_STATIC_LIBS ON)
    set(Qt5_USE_STATIC_RUNTIME ON)
    add_definitions(-DAXIOM_STATIC_BUILD=1)
    set(AXIOM_LINK_FLAGS -static -static-libgcc -static-libstdc++)
else ()
    set(AXIOM_LINK_FLAGS "")
endif ()

find_package(Qt5Widgets REQUIRED)
include_directories(../ ${Qt5Widgets_INCLUDE_DIRS})

set(RES_DIR "${CMAKE_SOURCE_DIR}/editor/resources")

# common settings for Mac bundles
set(ICON_PATH "${RES_DIR}/axiom.icns")

add_subdirectory(../common common)
add_subdirectory(model)
add_subdirectory(widgets)
add_subdirectory(compiler)
add_library(axiom_editor
        "${RES_DIR}/axiom.rc" "${RES_DIR}/res.qrc"
        util.h util.cpp
        AxiomApplication.h AxiomApplication.cpp
        AxiomEditor.h AxiomEditor.cpp
        backend/AudioBackend.h backend/AudioBackend.cpp
        backend/AudioConfiguration.h backend/AudioConfiguration.cpp
        backend/PersistentParameters.h backend/PersistentParameters.cpp)
target_link_libraries(axiom_editor axiom_widgets axiom_model axiom_common maxim_compiler Qt5::Widgets)

add_subdirectory(backend)

if (DEPLOY)
    set(CPACK_PACKAGE_NAME Axiom)
    set(CPACK_PACKAGE_VERSION 0.3.2)
    set(CPACK_PACKAGE_VENDOR MONAD)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Axiom: node-based synth")

    if (APPLE)
        set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)
        set(CPACK_GENERATOR DragNDrop)
    elseif (WIN32)
        set(CPACK_GENERATOR ZIP NSIS)
    elseif (UNIX)
        set(CPACK_GENERATOR TGZ)
    endif ()

    include(CPack)
endif ()

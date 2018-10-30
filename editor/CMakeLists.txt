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

# build RC file
configure_file("${RES_DIR}/axiom.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/axiom.rc" @ONLY)

# common settings for Windows executables
set(RC_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/axiom.rc")

# common settings for Mac bundles
set(ICON_PATH "${RES_DIR}/axiom.icns")

add_subdirectory(../common common)
add_subdirectory(model)
add_subdirectory(widgets)
add_subdirectory(compiler)
add_library(axiom_editor
        "${RES_DIR}/res.qrc"
        util.h util.cpp
        AxiomApplication.h AxiomApplication.cpp
        AxiomEditor.h AxiomEditor.cpp
        backend/AudioBackend.h backend/AudioBackend.cpp
        backend/AudioConfiguration.h backend/AudioConfiguration.cpp
        backend/PersistentParameters.h)
target_link_libraries(axiom_editor axiom_widgets axiom_model axiom_common maxim_compiler Qt5::Widgets)

add_subdirectory(backend)

if (DEPLOY)
    set(CPACK_PACKAGE_NAME Axiom)
    set(CPACK_PACKAGE_VERSION ${AXIOM_VERSION})
    set(CPACK_PACKAGE_VENDOR Monad)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Axiom: node-based synth")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
    set(CPACK_RESOURCE_FILE_README "${RES_DIR}/installer/README.txt")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY Axiom)
    set(CPACK_COMPONENT_STANDALONE_DISPLAY_NAME "Standalone")
    set(CPACK_COMPONENT_STANDALONE_DESCRIPTION "The standalone editor, which can be run without a DAW or host.")
    set(CPACK_COMPONENT_VSTEFFECT_DISPLAY_NAME "VST2 Effect")
    set(CPACK_COMPONENT_VSTEFFECT_DESCRIPTION "The VST2 effect, which runs in a DAW or host as an effect with audio input and output.")
    set(CPACK_COMPONENT_VSTINSTRUMENT_DISPLAY_NAME "VST2 Instrument")
    set(CPACK_COMPONENT_VSTINSTRUMENT_DESCRIPTION "The VST2 instrument, which runs in a DAW or host as an instrument with MIDI input and audio output.")

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE ${RES_DIR}/installer/README.txt DESTINATION .)

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

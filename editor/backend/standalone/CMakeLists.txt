find_package(PortAudio)

if (NOT PORTAUDIO_FOUND)
    message(WARNING "PortAudio could not be found, the standalone backend won't have audio output support.")
    set(PORTAUDIO_INCLUDE_DIRS "")
    set(PORTAUDIO_LIBRARIES "")
else ()
    message(STATUS "Found PortAudio in ${PORTAUDIO_LIBRARIES}")
    add_definitions(-DPORTAUDIO)

    # statically linking PortAudio on Windows needs setupapi.lib
    if (WIN32 AND AXIOM_STATIC_LINK)
        set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARIES} setupapi)
    endif ()
endif ()

set(AXIOM_STANDALONE_PROPERTIES)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(AXIOM_STANDALONE_PROPERTIES ${AXIOM_STANDALONE_PROPERTIES} WIN32 MACOSX_BUNDLE)
endif ()
include_directories(${PORTAUDIO_INCLUDE_DIRS})
set(ICON_PATH "${RES_DIR}/icon.icns")
add_executable(axiom_standalone ${AXIOM_STANDALONE_PROPERTIES} main.cpp "${ICON_PATH}")
set_source_files_properties("${ICON_PATH}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_target_properties(axiom_standalone PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${RES_DIR}/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "Axiom"
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.monadgroup.axiom"
    MACOSX_BUNDLE_ICON_FILE "Resources/icon.icns"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.3.2"
    MACOSX_BUNDLE_COPYRIGHT "Copyright 2018 MONAD")
target_link_libraries(axiom_standalone ${AXIOM_LINK_FLAGS} axiom_editor ${PORTAUDIO_LIBRARIES})
set_target_properties(axiom_standalone PROPERTIES
        PREFIX ""
        OUTPUT_NAME "AxiomStandalone")

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

find_package(PortMidi)

if (NOT PORTMIDI_FOUND)
    message(WARNING "PortMidi could not be found, the standalone backend won't have MIDI input support.")
    set(PORTMIDI_INCLUDE_DIRS "")
    set(PORTMIDI_LIBRARIES "")
else ()
    message(STATUS "Found PortMidi in ${PORTMIDI_LIBRARIES}")
    add_definitions(-DPORTMIDI)
endif ()

set(AXIOM_STANDALONE_PROPERTIES)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(AXIOM_STANDALONE_PROPERTIES ${AXIOM_STANDALONE_PROPERTIES} WIN32 MACOSX_BUNDLE)
endif ()
include_directories(${PORTAUDIO_INCLUDE_DIRS} ${PORTMIDI_INCLUDE_DIRS})
add_executable(axiom_standalone ${AXIOM_STANDALONE_PROPERTIES} main.cpp)
target_link_libraries(axiom_standalone ${PORTAUDIO_LIBRARIES} ${PORTMIDI_LIBRARIES})

add_backend(axiom_standalone "APPL" "Axiom Standalone" "" standalone FALSE)

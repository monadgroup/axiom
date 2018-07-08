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
    set(AXIOM_STANDALONE_PROPERTIES ${AXIOM_STANDALONE_PROPERTIES} WIN32)
endif ()

add_executable(axiom_standalone ${AXIOM_STANDALONE_PROPERTIES} main.cpp)
target_link_libraries(axiom_standalone axiom_editor ${PORTAUDIO_LIBRARIES})

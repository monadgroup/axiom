cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message("Reldeb flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_AUTOMOC ON)
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
find_package(PortAudio)

if (NOT PORTAUDIO_FOUND)
    message(WARNING "PortAudio could not be found, the standalone editor won't have audio output support.")
    set(PORTAUDIO_INCLUDE_DIRS "")
    set(PORTAUDIO_LIBRARIES "")
    set(AUDIO_SOURCES "StandaloneAudioNull.cpp")
else ()
    message(STATUS "Found Portaudio in ${PORTAUDIO_LIBRARIES}")
    add_definitions(-DPORTAUDIO)
    set(AUDIO_SOURCES "StandaloneAudioPA.cpp")

    # statically linking PortAudio on Windows needs setupapi.lib
    if (WIN32 AND AXIOM_STATIC_LINK)
        set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARIES} setupapi)
    endif ()
endif ()

if (NOT DEFINED VST2_SDK_ROOT)
    message(WARNING "VST2 SDK path was not specified, you won't be able to build the VST. In the future specify it with -DVST2_SDK_ROOT=/path/to/sdk")
else ()
    set(VST2_SOURCES
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/vstplugmain.cpp")
endif ()

set(SOURCE_FILES AxiomApplication.cpp util.cpp)
set(STANDALONE_SOURCE_FILES ${SOURCE_FILES} ${AUDIO_SOURCES} main.cpp)
set(VST_SOURCE_FILES ${SOURCE_FILES} AxiomVstEditor.cpp AxiomVstPlugin.cpp ${VST2_SOURCES})

set(RES_FILES resources/axiom.rc resources/res.qrc resources/AxiomVst.def)

include_directories(../ ${VST2_SDK_ROOT} ${PORTAUDIO_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})

set(LINK_LIBS axiom_widgets axiom_model axiom_common maxim_compiler Qt5::Widgets)

add_subdirectory(../common common)
add_subdirectory(model)
add_subdirectory(widgets)
add_subdirectory(compiler)

add_library(axiom_editor SHARED ${VST_SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom_editor ${AXIOM_LINK_FLAGS} ${LINK_LIBS})

set(AXIOM_STANDALONE_PROPERTIES )
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(AXIOM_STANDALONE_PROPERTIES ${AXIOM_STANDALONE_PROPERTIES} WIN32)
endif ()

add_executable(axiom_standalone ${AXIOM_STANDALONE_PROPERTIES} ${STANDALONE_SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom_standalone ${AXIOM_LINK_FLAGS} ${LINK_LIBS} ${PORTAUDIO_LIBRARIES})

set_target_properties(axiom_editor PROPERTIES
        BUNDLE true
        BUNDLE_EXTENSION "vst"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst"
        MACOSX_BUNDLE_INFO_PLIST "resources/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "Axiom: node-based VSTi"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.monadgroup.axiom"
        MACOSX_BUNDLE_ICON_FILE "resources/application.ico"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.2.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright 2018 MONAD")

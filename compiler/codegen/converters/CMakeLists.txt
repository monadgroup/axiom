cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AmplitudeConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/AmplitudeConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/BeatsConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/BeatsConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DbConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/DbConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/FrequencyConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/FrequencyConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoneConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoneConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoteConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoteConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/OscillatorConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/OscillatorConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/QConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/QConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SamplesConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/SamplesConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SecondsConverter.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/SecondsConverter.cpp")

target_sources(maxim_codegen PRIVATE ${SOURCE_FILES})

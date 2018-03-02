cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/BeatsConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DbConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/FrequencyConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/LinearConverter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SecondsConverter.cpp")

target_sources(maxim_codegen PRIVATE ${SOURCE_FILES})

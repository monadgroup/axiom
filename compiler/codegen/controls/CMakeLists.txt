cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ScalarControl.cpp")

target_sources(maxim_codegen PRIVATE ${SOURCE_FILES})

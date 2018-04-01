cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/HistoryList.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})

cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/NumComparisonOperator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumFloatOperator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumIntOperator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumIntrinsicOperator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumLogicalOperator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumOperator.cpp")

target_sources(maxim_codegen PRIVATE ${SOURCE_FILES})

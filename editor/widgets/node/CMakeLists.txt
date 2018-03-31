cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/CustomNodePanel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SyntaxHighlighter.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})

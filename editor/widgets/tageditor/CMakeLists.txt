cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/EntryFieldBox.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/EntryFieldInput.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/EntryFieldItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/TagLineEdit.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})

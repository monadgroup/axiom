cmake_minimum_required(VERSION 3.1)
project(axiom)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_VERBOSE_MAKEFILE ON)

message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message("Reldeb flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

# setup LTO linking
#set(CMAKE_AR "gcc-ar")
#set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
#set(CMAKE_CXX_ARCHIVE_FINISH true)

# setup Qt build
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCE_FILES src/main.cpp src/util.cpp src/AxiomApplication.cpp
        src/model/CustomNode.cpp src/model/Library.cpp src/model/LibraryEntry.cpp src/model/ModuleNode.cpp src/model/ModuleSchematic.cpp src/model/Node.cpp src/model/Project.cpp src/model/RootSchematic.cpp src/model/Schematic.cpp
        src/widgets/windows/MainWindow.cpp
        src/widgets/modulebrowser/ModuleBrowserPanel.cpp src/widgets/modulebrowser/ModulePreviewList.cpp src/widgets/modulebrowser/ModulePreview.cpp
        src/widgets/schematic/SchematicPanel.cpp src/widgets/schematic/SchematicCanvas.cpp
        src/widgets/node/NodeItem.cpp
        src/layouts/FlowLayout.cpp)

set(RES_FILES src/resources/axiom.rc src/resources/res.qrc)

add_executable(axiom WIN32 ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom Qt5::Widgets)

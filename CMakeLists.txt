cmake_minimum_required(VERSION 3.1)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
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
        src/model/Library.cpp src/model/LibraryEntry.cpp src/model/Project.cpp src/model/GridSurface.cpp src/model/GridItem.cpp
        src/model/connection/ConnectionSink.cpp src/model/connection/ConnectionSource.cpp src/model/connection/ConnectionWire.cpp
        src/model/node/CustomNode.cpp src/model/node/ModuleNode.cpp src/model/node/Node.cpp src/model/node/NodeSurface.cpp
        src/model/schematic/ModuleSchematic.cpp  src/model/schematic/RootSchematic.cpp src/model/schematic/Schematic.cpp
        src/model/control/NodeControl.cpp src/model/control/NodeValueControl.cpp src/model/control/NodeInputControl.cpp src/model/control/NodeOutputControl.cpp
        src/widgets/dock/DockPanel.cpp
        src/widgets/windows/MainWindow.cpp src/widgets/windows/AboutWindow.cpp
        src/widgets/modulebrowser/ModuleBrowserPanel.cpp src/widgets/modulebrowser/ModulePreviewList.cpp src/widgets/modulebrowser/ModulePreview.cpp
        src/widgets/schematic/SchematicPanel.cpp src/widgets/schematic/SchematicView.cpp src/widgets/schematic/SchematicCanvas.cpp
        src/widgets/node/NodeItem.cpp src/widgets/node/NodeResizer.cpp src/widgets/node/NodeItemContent.cpp
        src/widgets/controls/KnobControl.cpp
        src/layouts/FlowLayout.cpp)

set(RES_FILES src/resources/axiom.rc src/resources/res.qrc)

add_executable(axiom WIN32 ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom Qt5::Widgets)

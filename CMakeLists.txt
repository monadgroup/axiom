cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_VERBOSE_MAKEFILE ON)

message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message("Reldeb flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

# setup LTO linking
#set(CMAKE_AR "gcc-ar")
#set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
#set(CMAKE_CXX_ARCHIVE_FINISH true)

# setup Qt build
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCE_FILES editor/main.cpp editor/util.cpp editor/AxiomApplication.cpp
        editor/model/Library.cpp editor/model/LibraryEntry.cpp editor/model/Project.cpp editor/model/GridSurface.cpp editor/model/GridItem.cpp
        editor/model/connection/ConnectionSink.cpp editor/model/connection/NumConnectionSink.cpp editor/model/connection/MidiConnectionSink.cpp editor/model/connection/ConnectionWire.cpp
        editor/model/node/Node.cpp editor/model/node/CustomNode.cpp editor/model/node/ModuleNode.cpp editor/model/node/JunctionNode.cpp editor/model/node/NodeSurface.cpp
        editor/model/schematic/ModuleSchematic.cpp  editor/model/schematic/RootSchematic.cpp editor/model/schematic/Schematic.cpp
        editor/model/control/NodeControl.cpp editor/model/control/NodeNumControl.cpp

        editor/widgets/CommonColors.cpp editor/widgets/ItemResizer.cpp editor/widgets/FloatingValueEditor.cpp
        editor/widgets/dock/DockPanel.cpp
        editor/widgets/windows/MainWindow.cpp editor/widgets/windows/AboutWindow.cpp
        editor/widgets/modulebrowser/ModuleBrowserPanel.cpp editor/widgets/modulebrowser/ModulePreviewList.cpp editor/widgets/modulebrowser/ModulePreview.cpp
        editor/widgets/schematic/SchematicPanel.cpp editor/widgets/schematic/SchematicView.cpp editor/widgets/schematic/SchematicCanvas.cpp
        editor/widgets/node/NodeItem.cpp editor/widgets/node/CustomNodePanel.cpp editor/widgets/node/SyntaxHighlighter.cpp
        editor/widgets/controls/ControlItem.cpp editor/widgets/controls/NumControl.cpp
        editor/widgets/connection/WireItem.cpp
        editor/layouts/FlowLayout.cpp)

set(RES_FILES editor/resources/axiom.rc editor/resources/res.qrc)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(axiom WIN32 ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom Qt5::Widgets)

add_subdirectory(compiler)
target_link_libraries(axiom axiom_compiler)

llvm_map_components_to_libnames(llvm_libs support core irreader orcjit x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info x86utils)
target_link_libraries(axiom ${llvm_libs})


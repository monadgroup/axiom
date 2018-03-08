cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCE_FILES util.cpp AxiomApplication.cpp AxiomVstPlugin.cpp AxiomVstEditor.cpp
        model/Library.cpp model/LibraryEntry.cpp model/Project.cpp model/GridSurface.cpp model/GridItem.cpp
        model/connection/ConnectionSink.cpp model/connection/NumConnectionSink.cpp model/connection/MidiConnectionSink.cpp model/connection/ConnectionWire.cpp
        model/node/Node.cpp model/node/CustomNode.cpp model/node/ModuleNode.cpp model/node/OutputNode.cpp model/node/JunctionNode.cpp model/node/NodeSurface.cpp
        model/schematic/ModuleSchematic.cpp model/schematic/RootSchematic.cpp model/schematic/Schematic.cpp
        model/control/NodeControl.cpp model/control/NodeNumControl.cpp model/control/NodeMidiControl.cpp model/control/NodeOutputControl.cpp

        widgets/CommonColors.cpp widgets/ItemResizer.cpp widgets/FloatingValueEditor.cpp
        widgets/dock/DockPanel.cpp
        widgets/windows/MainWindow.cpp widgets/windows/AboutWindow.cpp
        widgets/modulebrowser/ModuleBrowserPanel.cpp widgets/modulebrowser/ModulePreviewList.cpp widgets/modulebrowser/ModulePreview.cpp
        widgets/schematic/SchematicPanel.cpp widgets/schematic/SchematicView.cpp widgets/schematic/SchematicCanvas.cpp
        widgets/node/NodeItem.cpp widgets/node/CustomNodePanel.cpp widgets/node/SyntaxHighlighter.cpp
        widgets/controls/painters/PlugPainter.cpp widgets/controls/painters/KnobPainter.cpp widgets/controls/painters/SliderPainter.cpp widgets/controls/painters/TogglePainter.cpp
        widgets/controls/ControlItem.cpp widgets/controls/NumControl.cpp widgets/controls/OutputControl.cpp
        widgets/connection/WireItem.cpp
        layouts/FlowLayout.cpp)

set(RES_FILES resources/axiom.rc resources/res.qrc resources/AxiomVst.def)

include_directories(../ ${LLVM_INCLUDE_DIRS} ../vendor/vst)
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader orcjit x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info x86utils)
set(LINK_LIBS maxim_runtime maxim_codegen maxim_common vst_sdk Qt5::Widgets ${llvm_libs})

add_subdirectory(../compiler compiler)
add_subdirectory(../vendor/vst vendor/vst)

add_library(axiom_editor SHARED ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom_editor ${LINK_LIBS})

add_executable(axiom_standalone main.cpp ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom_standalone ${LINK_LIBS})

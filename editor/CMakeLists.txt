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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(AXIOM_STATIC_LINK)
    set(Qt5_USE_STATIC_LIBS ON)
    set(Qt5_USE_STATIC_RUNTIME ON)
    add_definitions(-DAXIOM_STATIC_BUILD=1)
    set(AXIOM_LINK_FLAGS -static -static-libgcc -static-libstdc++)
else()
    set(AXIOM_LINK_FLAGS "")
endif()
find_package(Qt5Widgets REQUIRED)

set(SOURCE_FILES AxiomApplication.cpp util.cpp)
set(STANDALONE_SOURCE_FILES ${SOURCE_FILES} main.cpp)
set(VST_SOURCE_FILES ${SOURCE_FILES}
        AxiomVstEditor.cpp
        AxiomVstPlugin.cpp
        ../vendor/vst/public.sdk/source/vst2.x/audioeffect.cpp
        ../vendor/vst/public.sdk/source/vst2.x/audioeffectx.cpp
        ../vendor/vst/public.sdk/source/vst2.x/vstplugmain.cpp)

set(RES_FILES resources/axiom.rc resources/res.qrc resources/AxiomVst.def)

include_directories(../ ${LLVM_INCLUDE_DIRS} ../vendor/vst ${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core ipo irreader orcjit x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info x86utils)
set(LINK_LIBS axiom_widgets axiom_model maxim_runtime maxim_codegen maxim_common Qt5::Widgets ${llvm_libs})

add_subdirectory(model)
add_subdirectory(widgets)
add_subdirectory(../compiler compiler)

add_library(axiom_editor SHARED ${VST_SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom_editor ${LINK_LIBS} ${AXIOM_LINK_FLAGS})

add_executable(axiom_standalone ${STANDALONE_SOURCE_FILES} ${RES_FILES})
target_link_libraries(axiom_standalone ${LINK_LIBS} ${AXIOM_LINK_FLAGS})

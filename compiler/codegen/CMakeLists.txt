cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES
        Operator.cpp Value.cpp Builder.cpp MaximContext.cpp
        ModuleClass.cpp ComposableModuleClass.cpp
        ModuleClassMethod.cpp ComposableModuleClassMethod.cpp Function.cpp
        Control.cpp ControlField.cpp
        Array.cpp ArrayType.cpp
        Midi.cpp MidiType.cpp
        Num.cpp NumType.cpp
        Tuple.cpp TupleType.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(maxim_codegen ${SOURCE_FILES})

add_subdirectory(controls)
add_subdirectory(operators)
add_subdirectory(visitors)
target_link_libraries(maxim_codegen maxim_codegen_controls maxim_codegen_operators maxim_codegen_visitors)

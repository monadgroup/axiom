cmake_minimum_required(VERSION 3.4.3)
project(axiom_compiler)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOCC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES util.h
        common/FormType.cpp common/OperatorType.cpp common/ControlType.cpp
        parser/TokenStream.cpp parser/Token.cpp parser/Parser.cpp
        ast/Expression.cpp
        codegen/MaximContext.cpp codegen/Builder.cpp codegen/Instantiable.cpp codegen/Value.cpp codegen/Midi.cpp codegen/Num.cpp codegen/Tuple.cpp codegen/Array.cpp codegen/NumType.cpp codegen/MidiType.cpp codegen/TupleType.cpp codegen/ArrayType.cpp codegen/Converter.cpp codegen/Function.cpp codegen/Operator.cpp codegen/Control.cpp codegen/InstantiableFunction.cpp codegen/Node.cpp
        codegen/visitors/AssignVisitor.cpp codegen/visitors/CallVisitor.cpp codegen/visitors/CastVisitor.cpp codegen/visitors/ControlVisitor.cpp codegen/visitors/ExpressionVisitor.cpp codegen/visitors/MathVisitor.cpp codegen/visitors/NoteVisitor.cpp codegen/visitors/NumberVisitor.cpp codegen/visitors/PostfixVisitor.cpp codegen/visitors/TupleVisitor.cpp codegen/visitors/UnaryVisitor.cpp codegen/visitors/VariableVisitor.cpp codegen/visitors/LValueVisitor.cpp
        codegen/functions/VectorIntrinsicFunction.cpp codegen/functions/ScalarExternalFunction.cpp codegen/functions/VectorIntrinsicFoldFunction.cpp codegen/functions/ToRadFunction.cpp codegen/functions/ToDegFunction.cpp codegen/functions/ClampFunction.cpp codegen/functions/PanFunction.cpp codegen/functions/VectorShuffleFunction.cpp codegen/functions/CombineFunction.cpp codegen/functions/NoiseFunction.cpp codegen/functions/ActiveFunction.cpp codegen/functions/WithActiveFunction.cpp codegen/functions/NextFunction.cpp codegen/functions/DelayFunction.cpp codegen/functions/AmplitudeFunction.cpp codegen/functions/HoldFunction.cpp codegen/functions/AccumFunction.cpp
        codegen/operators/NumOperator.cpp codegen/operators/NumFloatOperator.cpp codegen/operators/NumIntrinsicOperator.cpp codegen/operators/NumIntOperator.cpp codegen/operators/NumComparisonOperator.cpp codegen/operators/NumLogicalOperator.cpp
        codegen/converters/LinearConverter.cpp codegen/converters/BeatsConverter.cpp codegen/converters/ControlConverter.cpp codegen/converters/DbConverter.cpp
        codegen/controls/NumberControl.cpp
        runtime/CompileLeaf.cpp runtime/CompileUnit.cpp runtime/Schematic.cpp runtime/RootSchematic.cpp runtime/ControlGroup.cpp runtime/Control.cpp runtime/HardControl.cpp runtime/SoftControl.cpp runtime/OutputControl.cpp runtime/Runtime.cpp runtime/Node.cpp runtime/CustomNode.cpp runtime/GroupNode.cpp runtime/OutputNode.cpp runtime/Jit.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(axiom_compiler ${SOURCE_FILES})
target_link_libraries(axiom_compiler Qt5::Widgets)
set_target_properties(axiom_compiler PROPERTIES LINKER_LANGUAGE CXX)

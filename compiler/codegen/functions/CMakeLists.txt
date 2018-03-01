cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES
        AccumFunction.cpp
        ActiveFunction.cpp
        AmplitudeFunction.cpp
        ClampFunction.cpp
        CombineFunction.cpp
        #DelayFunction.cpp
        HoldFunction.cpp
        MixFunction.cpp
        NextFunction.cpp
        NoiseFunction.cpp
        PanFunction.cpp
        PeriodicFunction.cpp
        RmpOscFunction.cpp
        SawOscFunction.cpp
        ScalarExternalFunction.cpp
        SequenceFunction.cpp
        SinOscFunction.cpp
        SqrOscFunction.cpp
        ToDegFunction.cpp
        ToRadFunction.cpp
        TriOscFunction.cpp
        VectorIntrinsicFoldFunction.cpp
        VectorIntrinsicFunction.cpp
        VectorShuffleFunction.cpp
        WithActiveFunction.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(maxim_codegen_functions ${SOURCE_FILES})

cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AccumFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ActiveFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/AmplitudeFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ClampFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CombineFunction.cpp"
       #"${CMAKE_CURRENT_SOURCE_DIR}/DelayFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/HoldFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MixFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NextFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoiseFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PanFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PeriodicFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RmpOscFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SawOscFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ScalarExternalFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SequenceFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SinOscFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SqrOscFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ToDegFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ToRadFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/TriOscFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VectorIntrinsicFoldFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VectorIntrinsicFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VectorShuffleFunction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/WithActiveFunction.cpp")

target_sources(maxim_codegen PRIVATE ${SOURCE_FILES})

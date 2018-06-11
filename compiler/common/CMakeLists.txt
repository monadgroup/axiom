cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        CompileError.h
        ControlDirection.h
        ControlType.h ControlType.cpp
        FormType.h FormType.cpp
        MidiEventType.h
        OperatorType.h OperatorType.cpp)

add_library(maxim_common ${SOURCE_FILES})

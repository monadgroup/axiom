set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Block.h" "${CMAKE_CURRENT_SOURCE_DIR}/Block.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ConstantValue.h" "${CMAKE_CURRENT_SOURCE_DIR}/ConstantValue.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlRef.h" "${CMAKE_CURRENT_SOURCE_DIR}/ControlRef.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Error.h" "${CMAKE_CURRENT_SOURCE_DIR}/Error.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Frontend.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Jit.h" "${CMAKE_CURRENT_SOURCE_DIR}/Jit.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeRef.h" "${CMAKE_CURRENT_SOURCE_DIR}/NodeRef.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/OwnedObject.h" "${CMAKE_CURRENT_SOURCE_DIR}/OwnedObject.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RootRef.h" "${CMAKE_CURRENT_SOURCE_DIR}/RootRef.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Runtime.h" "${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SurfaceRef.h" "${CMAKE_CURRENT_SOURCE_DIR}/SurfaceRef.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Transaction.h" "${CMAKE_CURRENT_SOURCE_DIR}/Transaction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ValueGroupSource.h" "${CMAKE_CURRENT_SOURCE_DIR}/ValueGroupSource.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VarType.h" "${CMAKE_CURRENT_SOURCE_DIR}/VarType.cpp")

target_sources(maxim_compiler PRIVATE ${SOURCE_FILES})
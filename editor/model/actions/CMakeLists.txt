set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Action.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CompositeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateConnectionAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateCustomNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateGroupNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreatePortalNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DeleteObjectAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MoveNodeAction.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})

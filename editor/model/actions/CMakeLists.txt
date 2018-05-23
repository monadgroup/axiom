set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Action.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CompositeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateConnectionAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateCustomNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateGroupNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateControlAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreatePortalNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DeleteObjectAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExposeControlAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/GridItemMoveAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/GridItemSizeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PasteBufferAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RenameNodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SetCodeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SetNumModeAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SetNumValueAction.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SetShowNameAction.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})

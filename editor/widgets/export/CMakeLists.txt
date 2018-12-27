set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AudioConfigWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/AudioConfigWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CodeConfigWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/CodeConfigWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/FileBrowserWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/FileBrowserWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MetaOutputConfigWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/MetaOutputConfigWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ObjectOutputConfigWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/ObjectOutputConfigWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PortalDefinitionEditorWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/PortalDefinitionEditorWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/TargetConfigWidget.h" "${CMAKE_CURRENT_SOURCE_DIR}/TargetConfigWidget.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})

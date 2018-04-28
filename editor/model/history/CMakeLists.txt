cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AddNodeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ChangeCodeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ChangeNumModeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ChangeNumValueOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ConnectControlsOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CreateNodeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DeleteNodeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DisconnectControlsOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/HistoryList.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/HistoryOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MoveControlOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MoveNodeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RenameNodeOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ShowHideControlNameOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SizeControlOperation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SizeNodeOperation.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})

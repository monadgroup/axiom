set(SOURCE_FILES
        CachedSequence.h
        CloneReferenceMapper.cpp
        ConnectionWire.cpp
        HistoryList.cpp
        IndexedSequence.h
        Library.cpp
        LibraryEntry.cpp
        ModelObject.cpp
        ModelRoot.cpp
        Pool.cpp
        PoolObject.cpp
        Project.cpp
        WireGrid.cpp
        Value.h)

include_directories(${QT5_INCLUDE_DIRS})

add_library(axiom_model ${SOURCE_FILES})

add_subdirectory(actions)
add_subdirectory(grid)
add_subdirectory(objects)
add_subdirectory(serialize)

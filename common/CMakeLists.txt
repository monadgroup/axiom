set(SOURCE_FILES Event.h
                 Promise.h
                 Sequence.h
                 SlotMap.h
                 TrackedObject.h TrackedObject.cpp)
add_library(axiom_common ${SOURCE_FILES})

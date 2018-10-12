set(SOURCE_FILES Cast.h
                 Event.h
                 Promise.h
                 Sequence.h
                 SlotMap.h
                 TrackedObject.h TrackedObject.cpp
                 WatchSequence.h)
add_library(axiom_common ${SOURCE_FILES})

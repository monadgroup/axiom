set(SOURCE_FILES Cast.h
                 Event.h
                 Promise.h
                 Sequence.h
                 SequenceOperators.h
                 SlotMap.h
                 TrackedObject.h TrackedObject.cpp
                 WatchSequence.h
                 WatchSequenceOperators.h)
add_library(axiom_common ${SOURCE_FILES})

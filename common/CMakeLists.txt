set(SOURCE_FILES Cast.h
                 Event.h
                 LazyInitializer.h
                 NamedLambda.h
                 Promise.h
                 Sequence.h
                 SequenceOperators.h
                 SingleIter.h
                 SlotMap.h
                 TrackedObject.h TrackedObject.cpp
                 WatchSequence.h
                 WatchSequenceOperators.h)
add_library(axiom_common ${SOURCE_FILES})

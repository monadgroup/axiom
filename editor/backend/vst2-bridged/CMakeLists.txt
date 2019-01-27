set(BRIDGED_QUEUE_SYNC_SOURCES "")

if (WIN32)
    set(BRIDGED_QUEUE_SYNC_SOURCES
            WinQueueSync.h WinQueueSync.cpp)
elseif (UNIX)
    set(BRIDGED_QUEUE_SYNC_SOURCES
            UnixQueueSync.h UnixQueueSync.cpp)
else ()
    message(WARNING "Platform is missing a queue sync implementation")
endif ()

set(VST2_BRIDGED_SHARED_SOURCES
        AppMessage.h
        Dispatcher.h
        IdBuffer.h IdBuffer.cpp
        SharedQueue.h
        VstChannel.h VstChannel.cpp
        VstMessage.h
        ${BRIDGED_QUEUE_SYNC_SOURCES})

set(VST2_BRIDGED_PLUGIN_SOURCES ${VST2_SOURCES}
                                ${VST2_BRIDGED_SHARED_SOURCES}
                                "../vst2-common/AxiomVst.def"
                                AxiomBridgedVstPlugin.h AxiomBridgedVstPlugin.cpp)

include_directories(${VST2_SDK_ROOT})

# VST2 Instrument
add_library(axiom_vst2_bridged_instrument MODULE ${VST2_BRIDGED_PLUGIN_SOURCES})
target_compile_definitions(axiom_vst2_bridged_instrument PRIVATE AXIOM_VST2_IS_SYNTH)
add_backend(axiom_vst2_bridged_instrument "BNDL" "Axiom VST2 Bridged Instrument" "vst" vstbridgedinstrument TRUE)

# VST2 Effect
add_library(axiom_vst2_bridged_effect MODULE ${VST2_BRIDGED_PLUGIN_SOURCES})
add_backend(axiom_vst2_bridged_effect "BNDL" "Axiom VST2 Bridged Effect" "vst" vstbridgedeffect TRUE)

# Bridged app
add_executable(axiom_vst2_bridged_app ${VST2_SHARED_SOURCES} ${VST2_BRIDGED_SHARED_SOURCES} app.cpp)
add_backend(axiom_vst2_bridged_app "APPL" "axiom_vst2_bridge" "" vst2_bridged FALSE)

# Test host
add_executable(axiom_vst2_bridged_test ${VST2_BRIDGED_SHARED_SOURCES} testhost.cpp)
add_backend(axiom_vst2_bridged_test "APPL" "axiom_vst2_bridge_test" "" vst2_bridged TRUE)

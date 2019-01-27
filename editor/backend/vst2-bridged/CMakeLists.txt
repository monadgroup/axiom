set(VST2_BRIDGED_SHARED_SOURCES
        AppMessage.h
        Dispatcher.h
        IdBuffer.h IdBuffer.cpp
        SharedQueue.h
        VstChannel.h
        VstMessage.h)

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

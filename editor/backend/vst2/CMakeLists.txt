set(VST2_BACKEND_SOURCES ${VST2_SHARED_SOURCES}
                         AxiomVstPlugin.h AxiomVstPlugin.cpp
                         AxiomVstEditor.h AxiomVstEditor.cpp)

include_directories(${VST2_SDK_ROOT})

# VST2 Instrument
add_library(axiom_vst2_instrument MODULE ${VST2_BACKEND_SOURCES})
target_compile_definitions(axiom_vst2_instrument PRIVATE AXIOM_VST2_IS_SYNTH)
add_backend(axiom_vst2_instrument "BNDL" "Axiom VST2 Instrument" "vst" vstinstrument FALSE)

# VST2 Effect
add_library(axiom_vst2_effect MODULE ${VST2_BACKEND_SOURCES})
add_backend(axiom_vst2_effect "BNDL" "Axiom VST2 Effect" "vst" vsteffect FALSE)

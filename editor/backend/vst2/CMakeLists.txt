if (NOT DEFINED VST2_SDK_ROOT)
    message(WARNING "VST2 SDK path was not specified, you won't be able to build the VST2 backend. In the future specify it with -DVST2_SDK_ROOT=/path/to/sdk")
else ()
    set(VST2_SOURCES
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp")
endif ()

include_directories(${VST2_SDK_ROOT})

set(VST2_BACKEND_SOURCES ${VST2_SOURCES}
                         AxiomVstPlugin.h AxiomVstPlugin.cpp
                         AxiomVstEditor.h AxiomVstEditor.cpp
                         VstAudioBackend.h VstAudioBackend.cpp
                         AxiomVst.def)

# VST2 Instrument
add_library(axiom_vst2_instrument MODULE ${VST2_BACKEND_SOURCES})
target_compile_definitions(axiom_vst2_instrument PRIVATE AXIOM_VST2_IS_SYNTH)
add_backend(axiom_vst2_instrument "BNDL" "Axiom VST2 Instrument" "vst" vstinstrument)

# VST2 Effect
add_library(axiom_vst2_effect MODULE ${VST2_BACKEND_SOURCES})
add_backend(axiom_vst2_effect "BNDL" "Axiom VST2 Effect" "vst" vsteffect)

if (NOT DEFINED VST2_SDK_ROOT)
    message(WARNING "VST2 SDK path was not specified, you won't be able to build the VST2 backend. In the future specify it with -DVST2_SDK_ROOT=/path/to/sdk")
else ()
    set(VST2_SOURCES
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp"
        PARENT_SCOPE)
endif ()

set(VST2_SHARED_SOURCES ${VST2_SOURCES}
                        "${CMAKE_CURRENT_SOURCE_DIR}/AxiomVst.def"
                        "${CMAKE_CURRENT_SOURCE_DIR}/VstAdapter.h"
                        "${CMAKE_CURRENT_SOURCE_DIR}/VstAudioBackend.h" "${CMAKE_CURRENT_SOURCE_DIR}/VstAudioBackend.cpp"
    PARENT_SCOPE)

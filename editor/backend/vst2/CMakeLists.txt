if (NOT DEFINED VST2_SDK_ROOT)
    message(WARNING "VST2 SDK path was not specified, you won't be able to build the VST2 backend. In the future specify it with -DVST2_SDK_ROOT=/path/to/sdk")
else ()
    set(VST2_SOURCES
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp")
endif ()

include_directories(${VST2_SDK_ROOT})
add_library(axiom_vst2 MODULE
        ${VST2_SOURCES}
        AxiomVstPlugin.h AxiomVstPlugin.cpp
        AxiomVstEditor.h AxiomVstEditor.cpp
        VstAudioBackend.h VstAudioBackend.cpp
        AxiomVst.def ${ICON_PATH})
set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set_target_properties(axiom_vst2 PROPERTIES
        PREFIX ""
        OUTPUT_NAME "AxiomVST"

        BUNDLE true
        BUNDLE_EXTENSION "vst"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst"
        MACOSX_BUNDLE_INFO_PLIST "${RES_DIR}/Info.plist.in"
        MACOSX_BUNDLE_INFO_STRING "Axiom VST2 0.3.2"
        MACOSX_BUNDLE_NAME "Axiom VST2")

target_link_libraries(axiom_vst2 ${AXIOM_LINK_FLAGS} axiom_editor)

install(TARGETS axiom_vst2 DESTINATION .)

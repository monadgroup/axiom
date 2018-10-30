if (NOT DEFINED VST2_SDK_ROOT)
    message(WARNING "VST2 SDK path was not specified, you won't be able to build the VST2 backend. In the future specify it with -DVST2_SDK_ROOT=/path/to/sdk")
else ()
    set(VST2_SOURCES
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp")
endif ()

include_directories(${VST2_SDK_ROOT})

set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set(VST2_BACKEND_SOURCES ${VST2_SOURCES}
        AxiomVstPlugin.h AxiomVstPlugin.cpp
        AxiomVstEditor.h AxiomVstEditor.cpp
        VstAudioBackend.h VstAudioBackend.cpp
        AxiomVst.def ${RC_FILE_PATH} ${ICON_PATH})

# VST2 Instrument
add_library(axiom_vst2_instrument MODULE ${VST2_BACKEND_SOURCES})
target_compile_definitions(axiom_vst2_instrument PRIVATE AXIOM_VST2_IS_SYNTH)
set_target_properties(axiom_vst2_instrument PROPERTIES
        PREFIX ""
        OUTPUT_NAME "Axiom VST2 Instrument"

        BUNDLE true
        BUNDLE_EXTENSION "vst"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst"
        MACOSX_BUNDLE_INFO_PLIST "${RES_DIR}/Info.plist.in"
        MACOSX_BUNDLE_INFO_STRING "Axiom VST2 Instrument ${AXIOM_VERSION}"
        MACOSX_BUNDLE_NAME "Axiom VST2 Instrument")
target_link_libraries(axiom_vst2_instrument ${AXIOM_LINK_FLAGS} axiom_editor)
install(TARGETS axiom_vst2_instrument
        DESTINATION .
        COMPONENT vstinstrument)

# VST2 Effect
add_library(axiom_vst2_effect MODULE ${VST2_BACKEND_SOURCES})
set_target_properties(axiom_vst2_effect PROPERTIES
        PREFIX ""
        OUTPUT_NAME "Axiom VST2 Effect"

        BUNDLE true
        BUNDLE_EXTENSION "vst"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst"
        MACOSX_BUNDLE_INFO_PLIST "${RES_DIR}/Info.plist.in"
        MACOSX_BUNDLE_INFO_STRING "Axiom VST2 Effect ${AXIOM_VERSION}"
        MACOSX_BUNDLE_NAME "Axiom VST2 Effect")
target_link_libraries(axiom_vst2_effect ${AXIOM_LINK_FLAGS} axiom_editor)
install(TARGETS axiom_vst2_effect
        DESTINATION .
        COMPONENT vsteffect)

if (AXIOM_PACK)
    add_custom_command(TARGET axiom_vst2_instrument POST_BUILD
                       COMMAND upx -9 --force "$<TARGET_FILE:axiom_vst2_instrument>")
    add_custom_command(TARGET axiom_vst2_effect POST_BUILD
                       COMMAND upx -9 --force "$<TARGET_FILE:axiom_vst2_effect>")
endif ()

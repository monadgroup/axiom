if (NOT DEFINED VST2_SDK_ROOT)
    message(WARNING "VST2 SDK path was not specified, you won't be able to build the VST2 backend. In the future specify it with -DVST2_SDK_ROOT=/path/to/sdk")
else ()
    set(VST2_SOURCES
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp"
            "${VST2_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp")
endif ()

add_library(axiom_vst2 SHARED ${VST2_SOURCES} ${RES_FILES} "${RES_DIR}/AxiomVst.def")
target_link_libraries(axiom_vst2 axiom_editor)

# todo: does this work?
set_target_properties(axiom_vst2 PROPERTIES
        BUNDLE true
        BUNDLE_EXTENSION "vst"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst"
        MACOSX_BUNDLE_INFO_PLIST "resources/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "Axiom: node-based VSTi"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.monadgroup.axiom"
        MACOSX_BUNDLE_ICON_FILE "resources/application.ico"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.3.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright 2018 MONAD")

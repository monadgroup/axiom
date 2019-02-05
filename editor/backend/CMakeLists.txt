function (add_backend_resources target bundle_type mini_mode)
    # generate PkgInfo file based on the bundle type
    set(PKGINFO_PATH "${CMAKE_CURRENT_BINARY_DIR}/PkgInfo")
    file(WRITE ${PKGINFO_PATH} "${bundle_type}????")

    target_sources(${target} PRIVATE ${PKGINFO_PATH} ${RC_FILE_PATH} ${ICON_PATH})
    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${PKGINFO_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION .)

    if (mini_mode)
        target_link_libraries(${target} ${AXIOM_LINK_FLAGS} axiom_backend Qt5::Core)
    else ()
        target_link_libraries(${target} ${AXIOM_LINK_FLAGS} axiom_editor)
    endif ()
endfunction ()

function (strip_target target)
    if (AXIOM_STRIP)
        if (APPLE)
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_STRIP} -x "$<TARGET_FILE:${target}>")
        else ()
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_STRIP} -s "$<TARGET_FILE:${target}>")
        endif ()
    endif ()
endfunction ()

function (pack_target target)
    if (AXIOM_PACK)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND upx -9 --force "$<TARGET_FILE:${target}>")
    endif ()
endfunction ()

function (add_backend target bundle_type output_name extension component mini_mode)
    add_backend_resources(${target} ${bundle_type} ${mini_mode})

    set_target_properties(${target} PROPERTIES
            PREFIX ""
            OUTPUT_NAME ${output_name}

            BUNDLE TRUE
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${RES_DIR}/Info.plist.in"
            MACOSX_BUNDLE_INFO_STRING "${output_name} ${AXIOM_VERSION}"
            MACOSX_BUNDLE_NAME ${output_name}
            MACOSX_BUNDLE_PACKAGE_TYPE ${bundle_type}
    )

    if (extension)
        set_target_properties(${target} PROPERTIES
                BUNDLE_EXTENSION ${extension}
                XCODE_ATTRIBUTE_WRAPPER_EXTENSION ${extension}
        )
    else ()
        set(extension "app")
    endif ()

    install(TARGETS ${target}
            DESTINATION .
            COMPONENT ${component})

    if (APPLE)
        install(CODE "
                set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
                set(Qt5Core_DIR ${Qt5Core_DIR})
                set(QT_LIBRARY_DIR ${QT_LIBRARY_DIR})
                set(QT_BINARY_DIR ${QT_BINARY_DIR})
                set(QT_QTCORE_LIBRARY_RELEASE ${QT_QTCORE_LIBRARY_RELEASE})
                set(QT_QTCORE_LIBRARY_DEBUG ${QT_QTCORE_LIBRARY_DEBUG})

                cmake_policy(SET CMP0009 NEW)
                cmake_policy(SET CMP0011 NEW)

                include(DeployQt5)
                fixup_qt5_executable(\"\${CMAKE_INSTALL_PREFIX}/${output_name}.${extension}\" ${extension} qcocoa \"\" \"\" \"\" \"\")
        " COMPONENT ${component})
    endif ()

    strip_target(${target})
    pack_target(${target})
endfunction ()

add_subdirectory(standalone)
add_subdirectory(vst2-common)
add_subdirectory(vst2)

if (AXIOM_VST2_BRIDGED_BETA)
    add_subdirectory(vst2-bridged)
endif ()

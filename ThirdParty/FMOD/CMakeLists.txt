set(FMOD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
    set(FMOD_CORE_INCLUDE ${FMOD_ROOT}/windows/api/core/inc)
    set(FMOD_STUDIO_INCLUDE ${FMOD_ROOT}/windows/api/studio/inc)
    set(FMOD_CORE_LIB ${FMOD_ROOT}/windows/api/core/lib/x64)
    set(FMOD_STUDIO_LIB ${FMOD_ROOT}/windows/api/studio/lib/x64)

    add_library(fmod SHARED IMPORTED GLOBAL)
    set_target_properties(fmod PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo"
            IMPORTED_IMPLIB_DEBUG "${FMOD_CORE_LIB}/fmodL_vc.lib"
            IMPORTED_LOCATION_DEBUG "${FMOD_CORE_LIB}/fmodL.dll"
            IMPORTED_IMPLIB_RELEASE "${FMOD_CORE_LIB}/fmod_vc.lib"
            IMPORTED_LOCATION_RELEASE "${FMOD_CORE_LIB}/fmod.dll"
            IMPORTED_IMPLIB_RELWITHDEBINFO "${FMOD_CORE_LIB}/fmod_vc.lib"
            IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_CORE_LIB}/fmod.dll"
            INTERFACE_INCLUDE_DIRECTORIES "${FMOD_CORE_INCLUDE}"
    )

    add_library(fmodstudio SHARED IMPORTED GLOBAL)
    set_target_properties(fmodstudio PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo"
            IMPORTED_IMPLIB_DEBUG "${FMOD_STUDIO_LIB}/fmodstudioL_vc.lib"
            IMPORTED_LOCATION_DEBUG "${FMOD_STUDIO_LIB}/fmodstudioL.dll"
            IMPORTED_IMPLIB_RELEASE "${FMOD_STUDIO_LIB}/fmodstudio_vc.lib"
            IMPORTED_LOCATION_RELEASE "${FMOD_STUDIO_LIB}/fmodstudio.dll"
            IMPORTED_IMPLIB_RELWITHDEBINFO "${FMOD_STUDIO_LIB}/fmodstudio_vc.lib"
            IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_STUDIO_LIB}/fmodstudio.dll"
            INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE}"
    )
    target_link_libraries(fmodstudio INTERFACE fmod)

elseif (UNIX AND NOT APPLE)
    set(FMOD_CORE_INC ${FMOD_ROOT}/linux/api/core/inc)
    set(FMOD_STUDIO_INC ${FMOD_ROOT}/linux/api/studio/inc)
    set(FMOD_CORE_LIBDIR ${FMOD_ROOT}/linux/api/core/lib/x86_64)
    set(FMOD_STUDIO_LIBDIR ${FMOD_ROOT}/linux/api/studio/lib/x86_64)

    add_library(fmod SHARED IMPORTED GLOBAL)
    set_target_properties(fmod PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo"
            IMPORTED_LOCATION_DEBUG "${FMOD_CORE_LIBDIR}/libfmodL.so"
            IMPORTED_LOCATION_RELEASE "${FMOD_CORE_LIBDIR}/libfmod.so"
            IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_CORE_LIBDIR}/libfmod.so"
            INTERFACE_INCLUDE_DIRECTORIES "${FMOD_CORE_INC}"
    )

    add_library(fmodstudio SHARED IMPORTED GLOBAL)
    set_target_properties(fmodstudio PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo"
            IMPORTED_LOCATION_DEBUG "${FMOD_STUDIO_LIBDIR}/libfmodstudioL.so"
            IMPORTED_LOCATION_RELEASE "${FMOD_STUDIO_LIBDIR}/libfmodstudio.so"
            IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_STUDIO_LIBDIR}/libfmodstudio.so"
            INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INC}"
    )
    target_link_libraries(fmodstudio INTERFACE fmod)

    foreach (_fmod_core_lib libfmod.so.14 libfmodL.so.14)
        if (NOT EXISTS "${FMOD_STUDIO_LIBDIR}/${_fmod_core_lib}")
            file(CREATE_LINK
                    "${FMOD_CORE_LIBDIR}/${_fmod_core_lib}"
                    "${FMOD_STUDIO_LIBDIR}/${_fmod_core_lib}"
                    SYMBOLIC
            )
        endif ()
    endforeach ()
endif ()

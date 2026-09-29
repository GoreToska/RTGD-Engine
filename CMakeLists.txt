cmake_minimum_required(VERSION 3.21)
project(RTGD-Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT Runtime)

include(InstallRequiredSystemLibraries)

option(RTGD_WITH_FMOD "Build audio with FMOD" ON)

# VS 18+ MSVC emits C5232 in SPIRV-Tools
if (MSVC)
    add_compile_options(/wd5232 /Wv:17)
endif ()

set(DILIGENT_NO_VULKAN OFF CACHE BOOL "" FORCE)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" RTGD_FMOD_PLATFORM)
if (RTGD_WITH_FMOD AND NOT EXISTS "${CMAKE_SOURCE_DIR}/ThirdParty/FMOD/${RTGD_FMOD_PLATFORM}")
    message(WARNING "FMOD SDK not found, building without audio")
    set(RTGD_WITH_FMOD OFF)
endif ()

add_subdirectory(ThirdParty/flecs)

if (TARGET flecs)
    target_compile_definitions(flecs PRIVATE flecs_EXPORTS)
endif ()

set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_UNITY_PLUGIN OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_VULKAN OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/DiligentEngine)

add_subdirectory(ThirdParty/spdlog)

add_subdirectory(ThirdParty/meshoptimizer)

set(GAINPUT_TESTS OFF CACHE BOOL "" FORCE)
set(GAINPUT_SAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/gainput)
add_subdirectory(ThirdParty/enkiTS)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/assimp)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(ThirdParty/json)

set(DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(GENERATE_DEBUG_SYMBOLS ON CACHE BOOL "" FORCE)
set(CROSS_PLATFORM_DETERMINISTIC OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(JPH_USE_VK OFF CACHE BOOL "" FORCE)
set(JPH_USE_DX12 OFF CACHE BOOL "" FORCE)
set(JPH_USE_MTL OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/JoltPhysics/Build)

if (RTGD_WITH_FMOD)
    add_subdirectory(ThirdParty/FMOD)
endif ()

add_subdirectory(Engine)
add_subdirectory(Game)


add_executable(RTGD-Engine
        main.cpp
)

set_target_properties(RTGD-Engine Engine Game PROPERTIES INSTALL_RPATH "$ORIGIN")

set(RTGD_THIRD_PARTY_TARGETS flecs gainput assimp
        Diligent-GraphicsEngineVk-shared Diligent-GraphicsEngineD3D12-shared)
set(RTGD_SHARED_THIRD_PARTY)
foreach (t IN LISTS RTGD_THIRD_PARTY_TARGETS)
    if (TARGET ${t})
        get_target_property(type ${t} TYPE)
        if (type STREQUAL "SHARED_LIBRARY")
            list(APPEND RTGD_SHARED_THIRD_PARTY ${t})
        endif ()
    endif ()
endforeach ()

install(TARGETS RTGD-Engine Engine Game ${RTGD_SHARED_THIRD_PARTY}
        RUNTIME_DEPENDENCY_SET RTGDRuntimeDeps
        RUNTIME DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION . COMPONENT Runtime
        ARCHIVE DESTINATION dev COMPONENT Dev)

install(RUNTIME_DEPENDENCY_SET RTGDRuntimeDeps
        DIRECTORIES $<TARGET_FILE_DIR:RTGD-Engine>
        PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
        POST_EXCLUDE_REGEXES ".*[Ss]ystem32/.*" "^/lib/.*" "^/lib64/.*" "^/usr/lib/.*"
        DESTINATION . COMPONENT Runtime)

install(DIRECTORY Assets Shaders DESTINATION . COMPONENT Runtime)

target_compile_definitions(RTGD-Engine PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_definitions(RTGD-Engine PRIVATE GAME_MODULE_NAME="$<TARGET_FILE_NAME:Game>")
set_target_properties(RTGD-Engine PROPERTIES WIN32_EXECUTABLE FALSE)

target_link_libraries(RTGD-Engine PRIVATE
        Engine
)

add_dependencies(RTGD-Engine Game)

add_custom_command(TARGET RTGD-Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:RTGD-Engine>/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:RTGD-Engine>/Shaders"
        COMMENT "Syncing assets and shaders"
        VERBATIM
)

if (WIN32)
    add_custom_command(TARGET RTGD-Engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:RTGD-Engine> "$<TARGET_FILE_DIR:RTGD-Engine>"
            COMMAND_EXPAND_LISTS
            VERBATIM
    )
endif ()
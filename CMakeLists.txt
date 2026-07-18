cmake_minimum_required(VERSION 3.21)
project(RTGD-Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# VS 18+ MSVC emits C5232 in SPIRV-Tools
if(MSVC)
    add_compile_options(/wd5232 /Wv:17)
endif()

set(DILIGENT_NO_VULKAN OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty/flecs)

set(DILIGENT_BUILD_SAMPLES      OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_DOCS         OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_UNITY_PLUGIN OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_VULKAN          OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/DiligentEngine)

add_subdirectory(ThirdParty/spdlog)

set(GAINPUT_TESTS   OFF CACHE BOOL "" FORCE)
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

add_subdirectory(Engine)
add_subdirectory(Game)


add_executable(RTGD-Engine
        main.cpp
)

target_compile_definitions(RTGD-Engine PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
set_target_properties(RTGD-Engine PROPERTIES WIN32_EXECUTABLE FALSE)

target_link_libraries(RTGD-Engine PRIVATE
        Engine
)

add_custom_command(TARGET RTGD-Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Assets"  "$<TARGET_FILE_DIR:RTGD-Engine>/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:RTGD-Engine>/Shaders"
        COMMENT "Syncing assets and shaders"
        VERBATIM
)

add_custom_command(TARGET RTGD-Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:RTGD-Engine> "$<TARGET_FILE_DIR:RTGD-Engine>"
        COMMAND_EXPAND_LISTS
        VERBATIM
)

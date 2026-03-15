cmake_minimum_required(VERSION 3.20)
project(RTGD-Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(ThirdParty/flecs)
add_subdirectory(ThirdParty/DiligentEngine)
add_subdirectory(ThirdParty/spdlog)
add_subdirectory(ThirdParty/gainput)

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
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Diligent-GraphicsEngineD3D12-shared>
        $<TARGET_FILE_DIR:RTGD-Engine>
)

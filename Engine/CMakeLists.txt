add_library(Engine SHARED
        Source/Engine/Engine.cpp
        Source/Engine/IEngineInterface.cpp
        Source/Engine/IGameModule.cpp
        Source/Render/RenderSystem.cpp
        Source/Render/ShaderLoader.cpp
        Include/Render/ShaderLoader.h
        Include/Engine/EngineExport.h
        Source/Engine/EngineExport.cpp
)

target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ThirdParty/flecs/include
)

target_link_libraries(Engine PUBLIC
        Diligent-GraphicsEngineVk-shared
        Diligent-Common
        Diligent-GraphicsEngineD3D12-shared
        Diligent-GraphicsTools
        flecs_static
)

target_compile_definitions(Engine PRIVATE ENGINE_EXPORTS NOMINMAX WIN32_LEAN_AND_MEAN)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Engine>
        "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net8.0-windows/Engine.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/bin/GraphicsEngineD3D12_64d.dll"
        "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net8.0-windows/GraphicsEngineD3D12_64d.dll"
)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders
        COMMENT "Copying shaders to output directory"
)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net8.0-windows/Shaders"
        COMMENT "Copying shaders to Editor directory"
)
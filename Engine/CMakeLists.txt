add_library(Engine SHARED
        Source/Engine/Engine.cpp
        Source/Engine/IEngineInterface.cpp
        Source/Engine/IGameModule.cpp
        Source/Render/RenderSystem.cpp
        Source/Render/ShaderLoader.cpp
        Include/Render/ShaderLoader.h
        Include/Engine/EngineExport.h
        Source/Engine/EngineExport.cpp
        Source/Tools/Logger.cpp
        Include/Tools/Logger.h
        Include/Tools/Alias.h
        Include/Render/Vertex.h
        Include/Components/MeshComponent.h
        Include/Components/RenderComponent.h
        Source/Render/RenderResourceManager.cpp
        Include/Render/RenderResourceManager.h
        Include/Render/RenderHandle.h
        Include/Render/SimpleMeshFactory.h
        Source/Render/SimpleMeshFactory.cpp
        Source/Scene/RTGDEntityFactory.cpp
        Source/Render/PipelineFactory.cpp
        Include/Render/PipelineFactory.h
        Source/Input/InputSystem.cpp
        Include/Input/InputSystem.h
        Include/Input/EInputAction.h
        Include/Components/CameraComponent.h
        Source/Systems/EditorCameraSystem.cpp
        Include/Systems/EditorCameraSystem.h
        Include/Components/VelocityComponent.h
        Source/Systems/MovementSystem.cpp
        Include/Systems/MovementSystem.h
        Source/Systems/CameraSystem.cpp
        Include/Systems/CameraSystem.h
        Include/Render/ConstBuffers.h
        Source/JobSystem/JobSystem.cpp
        Include/JobSystem/JobSystem.h
        Source/AssetLoader/AssetManager.cpp
        Include/AssetLoader/AssetManager.h
        Include/AssetLoader/MeshImporter.h
        Source/AssetLoader/MeshImporter.cpp
        Include/Tools/RTGDMacros.h
        Source/AssetLoader/TextureImporter.cpp
        Include/AssetLoader/TextureImporter.h
        Include/Components/LightComponent.h
        Source/Systems/LightSystem.cpp
        Include/Systems/LightSystem.h
        Include/Render/GBuffer.h
        Source/Render/GBufferFactory.cpp
        Include/Render/GBufferFactory.h
        Source/Scene/Scene.cpp
        Include/Scene/Scene.h
        Source/Scene/SceneManager.cpp
        Include/Scene/SceneManager.h
        Include/Tools/MetaTypes.h
        Source/Tools/MetaTypes.cpp
        Source/Engine/InspectorExports.cpp
        Include/Engine/InspectorExports.h
        Source/Platform/IPlatformWindow.cpp
        Include/Platform/IPlatformWindow.h
        Include/Platform/Linux/LinuxPlatformWindow.h
        Source/Platform/Linux/LinuxPlatformWindow.cpp
        Include/Platform/PlatformFactory.h
        Source/Platform/PlatformFactory.cpp
        Include/Platform/Windows/WindowsPlatformWindow.h
        Source/Platform/Windows/WindowsPlatformWindow.cpp
        Include/Platform/WindowHandle.h
        Include/Platform/WindowEvent.h
        Source/Platform/Linux/EmbeddedLinuxWindow.cpp
        Include/Platform/Linux/EmbeddedLinuxWindow.h
        Source/Input/KeyboardDevice.cpp
        Include/Input/KeyboardDevice.h
        Include/Input/InjectQueue.h
        Source/Input/InjectQueue.cpp
        Include/Input/IInjectable.h
        Source/Input/MouseDevice.cpp
        Include/Input/MouseDevice.h
        Source/Platform/Windows/EmbeddedWindowsWindow.cpp
        Include/Platform/Windows/EmbeddedWindowsWindow.h
        Include/Input/PhysicalKeyMap.h
        Include/AssetLoader/Refs/AssetRef.h
        Source/AssetLoader/Refs/AssetRef.cpp
        Source/Systems/TimerSystem.cpp
        Include/Systems/TimerSystem.h
        Include/AssetLoader/PathResolve.h
        Include/Event/EventBus.h
        Source/Event/EventBus.cpp
        Include/Event/Events.h
        Include/AssetLoader/AssetType.h
)

target_compile_definitions(Engine PRIVATE INSPECTOR_EXPORTS)

target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        ${CMAKE_SOURCE_DIR}/ThirdParty/gainput/lib/include
        ${CMAKE_SOURCE_DIR}/ThirdParty/stb
)

target_link_libraries(Engine PUBLIC
        Diligent-GraphicsEngineVk-shared
        Diligent-Common
        Diligent-GraphicsTools
        flecs_static
        spdlog::spdlog
        gainput
        assimp
        enkiTS
        nlohmann_json::nlohmann_json
)

if (WIN32)
    target_link_libraries(Engine PUBLIC
            Diligent-GraphicsEngineD3D12-shared
    )
elseif (UNIX AND NOT APPLE)
    target_link_libraries(Engine PUBLIC
            Diligent-GraphicsEngineVk-shared X11 Xi Xfixes
    )
endif ()

target_compile_definitions(Engine PRIVATE ENGINE_EXPORTS NOMINMAX WIN32_LEAN_AND_MEAN)

if (WIN32)
    add_custom_command(TARGET Engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Engine>
            "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net10.0/Engine.dll"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/GraphicsEngineD3D12_64d.dll"
            "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net10.0/GraphicsEngineD3D12_64d.dll"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:gainput>
            "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net10.0/$<TARGET_FILE_NAME:gainput>"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:assimp>
            "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net10.0/$<TARGET_FILE_NAME:assimp>"
    )
endif ()

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders
        COMMENT "Copying shaders to output directory"
)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets
        COMMENT "Copying Assets to output directory"
)


add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net10.0/Shaders"
        COMMENT "Copying shaders to Editor directory"
)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        "${CMAKE_SOURCE_DIR}/Editor/bin/Debug/net10.0/Assets"
        COMMENT "Copying assets to Editor directory"
)

set(EDITOR_OUT "${CMAKE_SOURCE_DIR}/Editor/bin/$<CONFIG>/net10.0")

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
        Include/Render/ResourcePool.h
        Source/Render/FrameConstants.cpp
        Include/Render/FrameConstants.h
        Include/Render/Graph/RenderContext.h
        Include/Render/Graph/Pass/IRenderPass.h
        Source/Render/Graph/RenderGraph.cpp
        Include/Render/Graph/RenderGraph.h
        Source/Render/Graph/Pass/GBufferPass.cpp
        Include/Render/Graph/Pass/GBufferPass.h
        Source/Render/Graph/Pass/LightPass.cpp
        Include/Render/Graph/Pass/LightPass.h
        Source/Render/Graph/Pass/CameraPass.cpp
        Include/Render/Graph/Pass/CameraPass.h
        Source/Render/Graph/Pass/DebugViewPass.cpp
        Include/Render/Graph/Pass/DebugViewPass.h
        Source/Render/Graph/RGResources.cpp
        Include/Render/Graph/RGResources.h
        Include/Render/Graph/RGBuilder.h
)

option(RTGD_EDITOR "Build engine with editor tools" ON)
set_target_properties(Engine PROPERTIES DEBUG_POSTFIX "")
set_target_properties(Engine PROPERTIES PREFIX "")

if (RTGD_EDITOR)
    target_compile_definitions(Engine PUBLIC RTGD_EDITOR)
endif ()

target_compile_definitions(Engine PRIVATE INSPECTOR_EXPORTS)

target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        ${CMAKE_SOURCE_DIR}/ThirdParty/gainput/lib/include
        ${CMAKE_SOURCE_DIR}/ThirdParty/stb
)

target_link_libraries(Engine PUBLIC
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
            Comctl32
    )
elseif (UNIX AND NOT APPLE)
    target_link_libraries(Engine PUBLIC
            Diligent-GraphicsEngineVk-shared
            X11
            Xi
            Xfixes
    )
endif ()

target_compile_definitions(Engine PRIVATE ENGINE_EXPORTS NOMINMAX WIN32_LEAN_AND_MEAN)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EDITOR_OUT}"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Engine> "${EDITOR_OUT}/$<TARGET_FILE_NAME:Engine>"

        COMMENT "Copying engine binary to the editor output directory"
        VERBATIM
)

if (WIN32)
    add_custom_command(TARGET Engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Engine> "${EDITOR_OUT}"
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime dependencies to the editor output directory"
            VERBATIM
    )
endif ()


if (WIN32 AND TARGET Diligent-GraphicsEngineD3D12-shared)
    add_custom_command(TARGET Engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Diligent-GraphicsEngineD3D12-shared>
            "${EDITOR_OUT}/$<TARGET_FILE_NAME:Diligent-GraphicsEngineD3D12-shared>"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Diligent-GraphicsEngineD3D12-shared>
            "$<TARGET_FILE_DIR:Engine>"

            COMMENT "Copying Diligent D3D12 engine DLL"
            VERBATIM
    )
elseif (UNIX AND NOT APPLE AND TARGET Diligent-GraphicsEngineVk-shared)
    add_custom_command(TARGET Engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Diligent-GraphicsEngineVk-shared>
            "${EDITOR_OUT}/$<TARGET_FILE_NAME:Diligent-GraphicsEngineVk-shared>"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Diligent-GraphicsEngineVk-shared>
            "$<TARGET_FILE_DIR:Engine>"

            COMMENT "Copying Diligent Vulkan engine library"
            VERBATIM
    )
endif ()

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:Engine>/Shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Assets"  "$<TARGET_FILE_DIR:Engine>/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Shaders" "${EDITOR_OUT}/Shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Assets"  "${EDITOR_OUT}/Assets"
        COMMENT "Syncing shaders and assets"
        VERBATIM
)
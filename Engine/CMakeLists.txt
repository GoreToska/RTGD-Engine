add_library(Engine SHARED
        Source/Engine/Engine.cpp
        Source/Engine/IEngineInterface.cpp
        Source/Engine/IGameModule.cpp
        Source/Render/RenderSystem.cpp
)

target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ThirdParty/flecs/include
)

target_link_libraries(Engine PRIVATE
        Magnum::Vk
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::Shaders
        Magnum::Trade
        # Corrade::Main
        flecs_static
)

target_compile_definitions(Engine PRIVATE ENGINE_EXPORTS NOMINMAX WIN32_LEAN_AND_MEAN)

add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Engine>
        ${CMAKE_SOURCE_DIR}/Editor/bin/Release/net8.0-windows/Engine.dll
        COMMENT "Copying Engine.dll to Editor"
)
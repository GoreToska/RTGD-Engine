add_library(Game SHARED
        Source/Game.cpp
        Source/dllmain.cpp
)

target_include_directories(Game PRIVATE
        ${CMAKE_SOURCE_DIR}/Engine/include
        ${CMAKE_SOURCE_DIR}/ThirdParty/flecs/include
)

target_link_libraries(Game PRIVATE
        Engine
        flecs_static
)

target_compile_definitions(Game PRIVATE GAME_EXPORTS)

# TODO: make copy for new editor
#[[
add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Game>
        ${CMAKE_SOURCE_DIR}/Editor/bin/Release/net8.0-windows/Game.dll
        COMMENT "Copying Game.dll to Editor"
)]]

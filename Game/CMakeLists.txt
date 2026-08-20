add_library(Game SHARED
        Source/Game.cpp
        Include/GameExpoty.h
)

target_include_directories(Game PRIVATE
        ${CMAKE_SOURCE_DIR}/Game/Include
        ${CMAKE_SOURCE_DIR}/Engine/Include
)

target_link_libraries(Game PRIVATE
        Engine
        flecs
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

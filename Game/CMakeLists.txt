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

add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:Game>/Shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:Game>/Assets"
        COMMENT "Syncing shaders and assets (Game)"
        VERBATIM
)

# TODO: make copy for new editor
#[[
add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Game>
        ${CMAKE_SOURCE_DIR}/Editor/bin/Release/net8.0-windows/Game.dll
        COMMENT "Copying Game.dll to Editor"
)]]

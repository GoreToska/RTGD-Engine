cmake_minimum_required(VERSION 3.20)
project(RTGD-Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Vulkan REQUIRED)

add_subdirectory(ThirdParty/flecs)

add_subdirectory(ThirdParty/corrade)

set(WITH_VULKAN ON CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/magnum)

set(WITH_VULKAN ON CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/magnum-integration)


add_subdirectory(Engine)
add_subdirectory(Game)

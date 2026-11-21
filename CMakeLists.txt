cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(collab)


file(GLOB_RECURSE SOURCE_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INCLUDE_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/include/*")

set(INCLUDES "")
set(SOURCES "")

foreach(ITEM ${SOURCE_DIRS})
    if(NOT IS_DIRECTORY ${ITEM})
        list(APPEND SOURCES ${ITEM})
    endif()
endforeach()

message(STATUS "Sources found: ${SOURCES}")

set(IMGUI_SOURCES
    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_sdlrenderer3.cpp
    
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp)

set(IMGUI_INCLUDES
    external/imgui/
    external/imgui/backends/)

message(STATUS "IMGUI sources: ${IMGUI_SOURCES}")
message(STATUS "IMGUI headers: ${IMGUI_INCLUDES}")
include_directories(${IMGUI_INCLUDES} ./external)

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

add_subdirectory(vk-bootstrap)

target_link_libraries(${PROJECT_NAME} PRIVATE  SDL3 glm)


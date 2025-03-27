cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

project(collab)

find_package(fmt REQUIRED)

file(GLOB_RECURSE SOURCE_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/src/*")
file(GLOB_RECURSE INCLUDE_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/include/*")

set(INCLUDES "")
set(SOURCES "")

foreach(ITEM ${SOURCE_DIRS})
    if(NOT IS_DIRECTORY ${ITEM})
        list(APPEND SOURCES ${ITEM})
    endif()
endforeach()

foreach(ITEM ${INCLUDE_DIRS})
    if(IS_DIRECTORY ${ITEM})
        list(APPEND INCLUDES ${ITEM})
    endif()
endforeach()

list(APPEND INCLUDES "${CMAKE_SOURCE_DIR}/include/")

message(STATUS "Sources found: ${SOURCES}")
message(STATUS "Include directories found: ${INCLUDES}")

set(IMGUI_SOURCES
    third_party/imgui/backends/imgui_impl_sdl3.cpp
    third_party/imgui/backends/imgui_impl_vulkan.cpp
    
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp)

set(IMGUI_INCLUDES
    third_party/imgui/
    third_party/imgui/backends/)

message(STATUS "IMGUI sources: ${IMGUI_SOURCES}")
message(STATUS "IMGUI headers: ${IMGUI_INCLUDES}")
include_directories(${INCLUDES} ${IMGUI_INCLUDES})

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

add_subdirectory(vk-bootstrap)

target_link_libraries(${PROJECT_NAME} PRIVATE fmt vulkan SDL3 vk-bootstrap::vk-bootstrap)




cmake_minimum_required(VERSION 3.0.0)

set(NAME Shutter CACHE INTERNAL "")
set(SHUTTER_VERSION "1.0")

project(${NAME} CXX)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

add_definitions(-std=c++11)

add_subdirectory (ShutterEngine)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/glfw)


set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/yaml-cpp)


set(SOURCE_GLOBAL ${SOURCE_GLOBAL} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp CACHE INTERNAL "")

add_executable(${NAME} ${SOURCE_GLOBAL})
target_link_libraries(${NAME} glfw)
target_link_libraries(${NAME} ${VULKAN_LIBRARY})
target_link_libraries(${NAME} yaml-cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/yaml-cpp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/imgui)
include_directories(${VULKAN_INCLUDE_DIR})

add_executable(${NAME}-Tools ${CMAKE_CURRENT_SOURCE_DIR}/Tools/main.cpp)
target_link_libraries(${NAME} yaml-cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/yaml-cpp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ShutterEngine/Ext/glm)
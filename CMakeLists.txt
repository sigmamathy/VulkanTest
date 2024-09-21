cmake_minimum_required(VERSION 3.29)
project(Test)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}

        src/main.cpp
        src/TestApp.cpp
        src/Graphics.cpp
        src/Drawing.cpp

        include/Dependencies.hpp
        include/TestApp.hpp
        include/Graphics.hpp
        include/Drawing.hpp
)

find_package(glfw3 CONFIG REQUIRED)

target_include_directories(Test PUBLIC include $ENV{VULKAN_SDK}/Include)
target_link_directories(Test PRIVATE $ENV{VULKAN_SDK}/Lib)
target_link_libraries(Test PRIVATE glfw vulkan-1)

# List of all shaders
set(SHADER_SOURCES

        "shader.vert"
        "shader.frag"
)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
endif()

# Compile all shaders using glslc upon each build
set(SHADER_TARGET_INDEX 0)
foreach(SHADER ${SHADER_SOURCES})
    add_custom_target("shader_build_${SHADER_TARGET_INDEX}"
            COMMAND glslc "${PROJECT_SOURCE_DIR}/shaders/${SHADER}" -o "${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv"
            COMMENT "Compile shader module ${SHADER}"
    )
    add_dependencies(${PROJECT_NAME} "shader_build_${SHADER_TARGET_INDEX}")
    math(EXPR SHADER_TARGET_INDEX "${SHADER_TARGET_INDEX} + 1")
endforeach()
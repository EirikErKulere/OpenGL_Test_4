cmake_minimum_required(VERSION 3.10)
project(OpenGL_Test_4 C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/loadTexture.cpp
    src/glad.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(include)

if(APPLE)
    set(HOMEBREW_PREFIX "/opt/homebrew")
    if (EXISTS "/usr/local/lib/libglfw.dylib")
        set(HOMEBREW_PREFIX "/usr/local")
    endif()

    message(STATUS "Using Homebrew prefix: ${HOMEBREW_PREFIX}")

    include_directories(${HOMEBREW_PREFIX}/include)
    link_directories(${HOMEBREW_PREFIX}/lib)

    find_package(assimp REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        "-L${HOMEBREW_PREFIX}/lib"
        glfw
        assimp::assimp
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif(UNIX)
    cmake_policy(SET CMP0072 NEW)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME}
        glfw
        ${OPENGL_gl_LIBRARY}
        dl
        pthread
    )
endif()

cmake_minimum_required(VERSION 3.15)

project(OpenGL_Test_4 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Shader.cpp
    src/loadTexture.cpp
    src/glad.c
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY resources DESTINATION share/${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE RESOURCE_PATH="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/resources"
)

include_directories(include)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    glfw
    OpenGL::GL
    assimp::assimp
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        dl
        pthread
    )
endif()

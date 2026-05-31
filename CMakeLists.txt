cmake_minimum_required(VERSION 3.18)
project(UltimateMotorcycle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---
find_package(SDL2 REQUIRED)
find_package(OpenGLES REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# --- Source files ---
file(GLOB_RECURSE ENGINE_SRC
    src/engine/*.cpp
    src/motorcycle/*.cpp
    src/world/*.cpp
    src/renderer/*.cpp
    src/audio/*.cpp
    src/input/*.cpp
    src/collision/*.cpp
)

set(ALL_SOURCES
    src/main.cpp
    ${ENGINE_SRC}
)

# --- Executable ---
add_executable(${PROJECT_NAME}
    ${ALL_SOURCES}
)

# --- Include directories ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${OPENGLES_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    src
)

# --- Link libraries ---
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${OPENGLES_LIBRARIES}
    ${GLEW_LIBRARIES}
)

# --- Compiler flags ---
if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DANDROID -Os")
    target_link_libraries(${PROJECT_NAME} log android)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")
endif()

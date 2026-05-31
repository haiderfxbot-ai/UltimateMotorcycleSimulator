cmake_minimum_required(VERSION 3.18)
project(UltimateMotorcycle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

if(ANDROID)
    # --- Android: download SDL2 + GLM from source ---
    include(FetchContent)

    FetchContent_Declare(SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.32.10
        GIT_SHALLOW TRUE
    )
    set(SDL2_SHARED ON CACHE BOOL "" FORCE)
    set(SDL2_STATIC OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2)

    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)

    # OpenGLES from NDK
    find_library(GLESv2_LIBRARY GLESv2 REQUIRED)
    find_library(EGL_LIBRARY EGL REQUIRED)
    find_library(log_LIBRARY log REQUIRED)
    find_library(android_LIBRARY android REQUIRED)

    set(DEP_LIBS SDL2-shared ${GLESv2_LIBRARY} ${EGL_LIBRARY} ${log_LIBRARY} ${android_LIBRARY})
    set(DEP_INCS ${SDL2_SOURCE_DIR}/include ${glm_SOURCE_DIR})
else()
    # --- Desktop: use system packages ---
    find_package(SDL2 REQUIRED)
    find_package(OpenGLES REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm REQUIRED)

    set(DEP_LIBS ${SDL2_LIBRARIES} ${OPENGLES_LIBRARIES} ${GLEW_LIBRARIES})
    set(DEP_INCS ${SDL2_INCLUDE_DIRS} ${OPENGLES_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
endif()

# --- Source files ---
file(GLOB_RECURSE ENGINE_SRC
    src/engine/*.cpp
    src/motorcycle/*.cpp
    src/world/*.cpp
    src/renderer/*.cpp
    src/audio/*.cpp
    src/input/*.cpp
    src/collision/*.cpp
    src/debug/*.cpp
)

set(ALL_SOURCES
    src/main.cpp
    ${ENGINE_SRC}
)

# --- Executable (desktop) / Shared library (Android) ---
if(ANDROID)
    add_library(${PROJECT_NAME} MODULE
        ${ALL_SOURCES}
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_NAME "main"
    )
else()
    add_executable(${PROJECT_NAME}
        ${ALL_SOURCES}
    )
endif()

# --- Include directories ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${DEP_INCS}
    src
)

# --- Link libraries ---
target_link_libraries(${PROJECT_NAME}
    ${DEP_LIBS}
)

# --- Compiler flags ---
if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DANDROID -Os -Wno-deprecated-declarations")
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--no-warn-mismatch)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")
endif()

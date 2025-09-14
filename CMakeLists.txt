cmake_minimum_required(VERSION 3.27.0 FATAL_ERROR)

project(
    lockstep
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Libraries
set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# SDL3 (3.2.20)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST_LIBRARY OFF)
set(SDL_GPU OFF)
set(SDL_AUDIO OFF)
set(SDL_CAMERA OFF)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.20
)
FetchContent_MakeAvailable(SDL3)

# GekkoNet
FetchContent_Declare(
    GekkoNet
    GIT_REPOSITORY https://github.com/HeatXD/GekkoNet
    GIT_TAG b1d0384d5181c35cbe0992825d9ba3841cf7ff6b
)
FetchContent_MakeAvailable(GekkoNet)

# S_fixed
set(S_FIXED_BUILD_TEST OFF)

FetchContent_Declare(
    S_fixed
    GIT_REPOSITORY https://github.com/Schwungus/S_fixed.git
    GIT_TAG master)
FetchContent_MakeAvailable(S_fixed)

set(LIBS SDL3::SDL3-static GekkoNet S_fixed)

# Build
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADERS ${SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
target_compile_definitions(${PROJECT_NAME} PUBLIC -D_POSIX_C_SOURCE=200112L)

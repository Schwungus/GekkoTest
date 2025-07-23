cmake_minimum_required(VERSION 3.27.0 FATAL_ERROR)

project(
    lockstep
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Libraries
set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# raylib (5.5)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

# ENet (2.6.5)
set(ENET_SHARED OFF)
set(ENET_STATIC ON)
set(ENET_TEST  "Build enet tests" ON)
set(ENET_USE_MORE_PEERS  "Build enet with up to 65k peers support" OFF)

FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/zpl-c/enet.git
    GIT_TAG v2.6.5
)
FetchContent_MakeAvailable(enet)

set(LIBS raylib enet::enet_static)

# Build
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADERS ${SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
# add_custom_command(
#     TARGET ${PROJECT_NAME}
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#     COMMAND_EXPAND_LISTS
# )

# Copy assets
# set(MOD_SOURCE_DIR ${CMAKE_SOURCE_DIR}/modsrc)
# add_custom_command(
#     TARGET ${PROJECT_NAME}
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${MOD_SOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
# )

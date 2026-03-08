cmake_minimum_required(VERSION 3.16)
project(SDFDrops)

# set the output directory for built objects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

file(GLOB SOURCES "${INCLUDE_DIR}/*.c")

# Create your game executable target as a **console** app (remove WIN32)
add_executable(SDFDrops main.c ${SOURCES})

target_include_directories(SDFDrops
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

# Link to SDL3
target_link_libraries(SDFDrops PRIVATE SDL3::SDL3)

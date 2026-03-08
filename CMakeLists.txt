cmake_minimum_required(VERSION 3.16)
project(SDFDrops)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)


file(GLOB SOURCES "${INCLUDE_DIR}/*.c")

# Create your game executable target as usual
add_executable(SDFDrops WIN32 main.c ${SOURCES})

target_include_directories(SDLTest
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

# Link to the actual SDL3 library.
target_link_libraries(SDFDrops PRIVATE SDL3::SDL3)

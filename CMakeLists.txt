cmake_minimum_required(VERSION 3.21)
project(Game_Engine12)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically download and integrate vcpkg if not already available
set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg")
if(NOT EXISTS "${VCPKG_DIR}")
    message(STATUS "Cloning vcpkg into ${VCPKG_DIR}")
    include(FetchContent)
    FetchContent_Declare(
        vcpkg
        GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
        GIT_TAG        2024.05.24 # Replace with latest stable release if needed
    )
    FetchContent_MakeAvailable(vcpkg)
endif()

# Enable vcpkg manifest mode
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Add source files and include directories
file(GLOB_RECURSE SOURCES "GameEngine/*.cpp")
file(GLOB_RECURSE HEADERS "GameEngine/*.h" "GameEngine/*.hpp")

add_executable(Game_Engine12 ${SOURCES} ${HEADERS})

# Include GameEngine directory
target_include_directories(Game_Engine12 PRIVATE "${CMAKE_SOURCE_DIR}/GameEngine")

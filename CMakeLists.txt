cmake_minimum_required(VERSION 3.21)
project(MyDX12Engine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Run custom bootstrap logic
include("${CMAKE_CURRENT_LIST_DIR}/scripts/bootstrap-vcpkg.cmake")

# Enable vcpkg manifest mode
set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Add source files and include directories
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")

add_executable(MyDX12Engine ${SOURCES} ${HEADERS})

# Optional includes / linking
# target_include_directories(MyDX12Engine PRIVATE "src")
# target_link_libraries(MyDX12Engine PRIVATE SomeLibrary)
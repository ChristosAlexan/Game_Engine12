cmake_minimum_required(VERSION 3.21)
project(GameEngine12)

# Auto-bootstrap vcpkg if missing
include(${CMAKE_SOURCE_DIR}/scripts/bootstrap-vcpkg.cmake)

# Setup vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# Use C++17 or later
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable vcpkg manifest mode
set(VCPKG_MANIFEST_MODE ON)

# Enable folders in IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add source files
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
    src/*.hpp
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add the executable
add_executable(${PROJECT_NAME} ${ENGINE_SRC})

# vcpkg dependencies
find_package(SDL3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(polyclipping CONFIG REQUIRED)
find_package(zlib CONFIG REQUIRED)
find_package(stb CONFIG REQUIRED)
find_package(utfcpp CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED)
find_package(kubazip CONFIG REQUIRED)
find_package(rapidjson CONFIG REQUIRED)
find_package(draco CONFIG REQUIRED)
find_package(jhasse-poly2tri CONFIG REQUIRED)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3
        nlohmann_json::nlohmann_json
        pugixml
        polyclipping
        ZLIB::ZLIB
        stb::stb
        utfcpp::utfcpp
        minizip::minizip
        kubazip::kubazip
        rapidjson
        draco::draco
        poly2tri::poly2tri
)

# Optional: organize source files into IDE folders
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${ENGINE_SRC})
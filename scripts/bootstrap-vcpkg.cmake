# This script checks if vcpkg is present and bootstraps it if needed

set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")

if(NOT EXISTS "${VCPKG_ROOT}/vcpkg")
    message(STATUS "Cloning vcpkg...")
    execute_process(COMMAND git clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
                    RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to clone vcpkg")
    endif()
endif()

if(NOT EXISTS "${VCPKG_ROOT}/vcpkg.exe")
    message(STATUS "Bootstrapping vcpkg...")
    if(WIN32)
        execute_process(COMMAND cmd /c bootstrap-vcpkg.bat
                        WORKING_DIRECTORY "${VCPKG_ROOT}"
                        RESULT_VARIABLE result)
    else()
        execute_process(COMMAND ./bootstrap-vcpkg.sh
                        WORKING_DIRECTORY "${VCPKG_ROOT}"
                        RESULT_VARIABLE result)
    endif()

    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to bootstrap vcpkg")
    endif()
endif()
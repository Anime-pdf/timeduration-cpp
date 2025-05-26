cmake_minimum_required(VERSION 3.14)
project(timeduration-cpp VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TIMEDURATION_BUILD_EXAMPLES "Build example applications" OFF)
option(TIMEDURATION_BUILD_TESTS "Build tests" OFF)
option(TIMEDURATION_DOWNLOAD_GTEST "Download Google Test if not found" ON)

if(TIMEDURATION_BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

add_library(timeduration INTERFACE)
add_library(timeduration::timeduration ALIAS timeduration)

target_include_directories(timeduration
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(
        DIRECTORY include/
        DESTINATION include
)
install(
        TARGETS timeduration
        EXPORT timeduration-targets
        INCLUDES DESTINATION include
)
install(
        EXPORT timeduration-targets
        FILE timeduration-targets.cmake
        NAMESPACE timeduration::
        DESTINATION lib/cmake/timeduration
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/timeduration-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/timeduration-config.cmake
        INSTALL_DESTINATION lib/cmake/timeduration
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/timeduration-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/timeduration-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/timeduration-config-version.cmake
        DESTINATION lib/cmake/timeduration
)

if(TIMEDURATION_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(TIMEDURATION_BUILD_TESTS)
    find_package(GTest QUIET)

    if(NOT GTest_FOUND AND TIMEDURATION_DOWNLOAD_GTEST)
        message(STATUS "Google Test not found. Downloading...")
        include(FetchContent)
        FetchContent_Declare(
                googletest
                GIT_REPOSITORY https://github.com/google/googletest.git
                GIT_TAG v1.14.0  # Specify a specific tag/version
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(googletest)
    endif()

    add_subdirectory(tests)
endif()

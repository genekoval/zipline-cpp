cmake_minimum_required(VERSION 3.27)

include(cmake/modules.cmake)
include(GitVersion)

project(zipline
    VERSION ${GIT_VERSION_ABBREV}
    DESCRIPTION "Zipline serialization and protocol utilities for C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)

include(ProjectTesting)
include(packages.cmake)

add_library(zipline)
add_library(${PROJECT_NAME}::zipline ALIAS zipline)
target_sources(zipline PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_link_libraries(zipline
    PUBLIC
        ext::ext
        fmt::fmt
        timber::timber
)

if(PROJECT_TESTING)
    add_executable(zipline.test)
    target_link_libraries(zipline.test
        PRIVATE
            GTest::gtest_main
            netcore::netcore
            zipline::zipline
    )
    add_test(
        NAME "Unit Tests"
        COMMAND zipline.test
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
endif()

add_subdirectory(include)
add_subdirectory(src)

include(Sanitizers)
enable_asan()

include(EasyInstall)
easy_install(
    TARGETS zipline
    DEPENDENCIES
        ext
        fmt
        timber
)

if(PROJECT_IS_TOP_LEVEL)
    add_custom_target(docker
        docker build --progress=plain .
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        USES_TERMINAL
    )
endif()


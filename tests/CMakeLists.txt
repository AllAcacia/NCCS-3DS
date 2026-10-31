
cmake_minimum_required(VERSION 3.20)

project(NCCS3DS_Tests LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable CTest
include(CTest)
enable_testing()

# Unity test framework
add_library(unity STATIC
    unity/src/unity.c
)

target_include_directories(unity PUBLIC
    unity/src
)

# Test executable
add_executable(nccs_tests
    test_nccs.c
)

# Link Unity
target_link_libraries(nccs_tests PRIVATE
    unity
)

# Include production headers
target_include_directories(nccs_tests PRIVATE
    ../inc
    ../src
)

# Register tests
add_test(
    NAME NCCS3DS_UnitTests
    COMMAND nccs_tests
)

cmake_minimum_required(VERSION 3.10)
project(VL53L1 VERSION 1.0 LANGUAGES C CXX)

include_directories(
    core/inc
    platform/inc
)

file(GLOB_RECURSE CORE_SOURCES core/src/*.[cC])
file(GLOB_RECURSE PLATFORM_SOURCES platform/src/*.[cC])

add_library(VL53L1 STATIC
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(VL53L1
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platform/inc>
)



target_compile_features(VL53L1 PRIVATE cxx_std_17)
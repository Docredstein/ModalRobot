cmake_policy(SET CMP0048 NEW)
project(testVideo VERSION 1.0.1)
cmake_minimum_required(VERSION 3.0.0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_GENERATOR Ninja)
unset (DEBUG_SCREEN)
set(CMAKE_BUILD_TYPE Debug)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

include_directories(${LIBCAMERA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})

add_subdirectory(./MotorControl)
add_subdirectory(./PID)
add_subdirectory(./Encoder)
add_subdirectory(./Holonomic)
add_executable(testVideo main.cpp)

#target_link_libraries(testVideo PRIVATE MotorControl)
target_link_libraries(testVideo -llccv ${OpenCV_LIBS} MotorControl PID Encoder Holonomic -lwiringPi)


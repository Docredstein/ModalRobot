cmake_minimum_required(VERSION 3.10)
project(MotorControl VERSION 1.0 LANGUAGES CXX)

add_library(MotorControl MotorControl.cpp)
target_include_directories(MotorControl PUBLIC .)
target_compile_features(MotorControl PRIVATE cxx_std_17)
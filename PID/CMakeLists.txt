cmake_minimum_required(VERSION 3.10)
project(PID VERSION 1.0 LANGUAGES CXX)

add_library(PID PID.cpp)
target_include_directories(PID PUBLIC .)
target_compile_features(PID PRIVATE cxx_std_17)
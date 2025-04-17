cmake_minimum_required(VERSION 3.10)
project(Holonomic VERSION 1.0 LANGUAGES CXX)

add_library(Holonomic Holonomic.cpp)
target_include_directories(Holonomic PUBLIC .)
target_compile_features(Holonomic PRIVATE cxx_std_17)
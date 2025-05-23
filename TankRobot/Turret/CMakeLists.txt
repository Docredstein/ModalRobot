cmake_minimum_required(VERSION 3.10)
project(Turret VERSION 1.0 LANGUAGES CXX)

add_library(Turret Turret.cpp)
target_include_directories(Turret PUBLIC .)
target_compile_features(Turret PRIVATE cxx_std_17)
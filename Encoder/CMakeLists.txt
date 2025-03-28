cmake_minimum_required(VERSION 3.10)
project(Encoder VERSION 1.0 LANGUAGES CXX)

add_library(Encoder Encoder.cpp)
target_include_directories(Encoder PUBLIC .)
target_compile_features(Encoder PRIVATE cxx_std_17)
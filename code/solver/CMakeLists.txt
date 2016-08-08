cmake_minimum_required (VERSION 3.1.0)

project (solver)

add_executable(solve main.cpp)

target_compile_features(solve PRIVATE cxx_range_for)

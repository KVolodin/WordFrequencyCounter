cmake_minimum_required(VERSION 3.16)
project(freq)

add_executable(${PROJECT_NAME} src/freq.cpp)

target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE include)

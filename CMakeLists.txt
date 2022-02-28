cmake_minimum_required(VERSION 3.16)
project(freq)

option(MAPPED_FILE_BOOST "mapped file from boost lib" OFF)

if (MAPPED_FILE_BOOST)
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost REQUIRED COMPONENTS iostreams)
endif (MAPPED_FILE_BOOST)

add_executable(${PROJECT_NAME} src/freq.cpp)

target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE include)

if (MAPPED_FILE_BOOST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAPPED_FILE_BOOST)
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif (MAPPED_FILE_BOOST)
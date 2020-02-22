cmake_minimum_required(VERSION 3.10)

project(png2protobuf LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PNG REQUIRED)

add_executable(png2protobuf
  src/main.cpp
)

target_include_directories(png2protobuf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_libraries(png2protobuf PRIVATE PNG::PNG)

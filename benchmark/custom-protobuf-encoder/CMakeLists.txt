cmake_minimum_required(VERSION 3.10)

project(custom-protobuf-encoder LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PNG REQUIRED)

add_executable(custom-protobuf-encoder
  custom-protobuf-encoder.cpp
)

target_link_libraries(custom-protobuf-encoder PRIVATE PNG::PNG)

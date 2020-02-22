cmake_minimum_required(VERSION 3.10)

project(tensorflow-example-encoder LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PNG REQUIRED)

add_executable(tensorflow-example-encoder
  src/main.cpp
)

target_include_directories(tensorflow-example-encoder PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_libraries(tensorflow-example-encoder PRIVATE PNG::PNG)

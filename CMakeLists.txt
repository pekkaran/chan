cmake_minimum_required(VERSION 3.5)
project(chan LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(chan)

add_executable(chan_test chan/test.c)
target_link_libraries(chan_test PRIVATE chan)

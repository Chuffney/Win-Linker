cmake_minimum_required(VERSION 3.20)
project(Linker C)

set(CMAKE_C_STANDARD 99)

include_directories(include)
add_executable(Linker main.c resizeableBuffer.obj emission.c)

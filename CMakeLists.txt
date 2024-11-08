cmake_minimum_required(VERSION 3.22)
# find_package(JetsonGPIO)
project(test)
set(CMAKE_BUILD_TYPE "Debug")

add_subdirectory(JetsonGPIO)
add_executable(main main.cpp)
target_link_libraries(main JetsonGPIO)
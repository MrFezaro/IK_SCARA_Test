cmake_minimum_required(VERSION 3.30)
project(IK_SCARA_Test)

set(CMAKE_CXX_STANDARD 26)

add_subdirectory(src)

add_executable(IK_SCARA_Test main.cpp)
target_link_libraries(IK_SCARA_Test PRIVATE scara_lib)
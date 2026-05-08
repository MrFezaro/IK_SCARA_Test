add_library(scara_lib IK.cpp GUI.cpp animation.cpp)
target_include_directories(scara_lib PUBLIC "${CMAKE_SOURCE_DIR}/include")
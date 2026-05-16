add_library(scara_lib IK.cpp GUI.cpp animation.cpp)
target_include_directories(scara_lib PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(scara_lib PUBLIC SerialComm::SerialComm)
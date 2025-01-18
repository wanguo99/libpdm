# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(PDM)

add_subdirectory(lib)
add_subdirectory(app/pdm_test)
add_subdirectory(app/websocket_server)
add_subdirectory(web)

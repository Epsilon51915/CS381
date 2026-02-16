cmake_minimum_required(VERSION 3.18)
project(as2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(../raylib-cpp raylib)

file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as2 src/as2.cpp)
target_link_libraries(as2 PUBLIC raylib raylib_cpp)
set_property(TARGET as2 PROPERTY CXX_STANDARD 20)

configure_file(../assets/models/penguin.glb models/penguin.glb COPYONLY)
configure_file(../assets/models/eagle.glb models/eagle.glb COPYONLY)
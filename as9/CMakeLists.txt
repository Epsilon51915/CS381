cmake_minimum_required(VERSION 3.18)
project(as9 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(../raylib-cpp raylib)

include(../assets/includeable.cmake)

file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as9 src/as9.cpp src/componentCounter.cpp)
target_link_libraries(as9 PUBLIC raylib raylib_cpp raygui)
set_property(TARGET as9 PROPERTY CXX_STANDARD 20)
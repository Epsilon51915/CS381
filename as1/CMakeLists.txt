cmake_minimum_required(VERSION 3.18)
project(as1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(../raylib-cpp raylib)

file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as1 src/as1.cpp src/raygui.cpp)
target_link_libraries(as1 PUBLIC raylib raylib_cpp raygui)
set_property(TARGET as1 PROPERTY CXX_STANDARD 20)

configure_file(../assets/audio/fah.mp3 audio/fah.mp3 COPYONLY)
configure_file(../assets/audio/ping.mp3 audio/ping.mp3 COPYONLY)
configure_file(../assets/audio/fred.mp3 audio/fred.mp3 COPYONLY)
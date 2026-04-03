cmake_minimum_required(VERSION 3.18)
project(as7 CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(../raylib-cpp raylib)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ../asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

include(../assets/includeable.cmake)


add_executable(as7 src/as7.cpp)
target_link_libraries(as7 asio)
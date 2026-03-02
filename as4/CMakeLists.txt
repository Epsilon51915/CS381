cmake_minimum_required(VERSION 3.18)
project(as4 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(../raylib-cpp raylib)

include(../assets/includeable.cmake)

file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as4 src/as4.cpp src/skybox.cpp)
target_link_libraries(as4 PUBLIC raylib raylib_cpp raygui)
set_property(TARGET as4 PROPERTY CXX_STANDARD 20)

make_includeable(../assets/shaders/skybox.vs generated/skybox.vs)
make_includeable(../assets/shaders/skybox.fs generated/skybox.fs)
make_includeable(../assets/shaders/cubemap.vs generated/cubemap.vs)
make_includeable(../assets/shaders/cubemap.fs generated/cubemap.fs)

configure_file(../assets/models/penguin.glb models/penguin.glb COPYONLY)
configure_file(../assets/models/eagle.glb models/eagle.glb COPYONLY)
configure_file(../assets/textures/skybox.png textures/skybox.png COPYONLY)
configure_file(../assets/textures/snow.jpg textures/snow.jpg COPYONLY)
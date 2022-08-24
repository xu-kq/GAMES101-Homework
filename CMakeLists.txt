cmake_minimum_required(VERSION 3.10)


# vcpkg 
find_path(vcpkg_path vcpkg.exe)
set(CMAKE_TOOLCHAIN_FILE ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake)


set(CMAKE_CXX_STANDARD 17)
project(Rasterizer)
add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp global.hpp Triangle.hpp Triangle.cpp Texture.hpp Texture.cpp Shader.hpp OBJ_Loader.h)


# eigen
find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(Rasterizer  ${Eigen3_LIBS})


# opencv4
find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(Rasterizer  ${OpenCV_LIBS})

#target_compile_options(Rasterizer PUBLIC -Wall -Wextra -pedantic)
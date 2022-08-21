cmake_minimum_required(VERSION 3.10)
project(Rasterizer)


set(CMAKE_CXX_STANDARD 17)

add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)

# vcpkg 
find_path(vcpkg_path vcpkg.exe)
include(${vcpkg_path}/scripts/buildsystems/vcpkg.cmake)
# set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
# include(${CMAKE_TOOLCHAIN_FILE})

# eigen
find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(Rasterizer  ${Eigen3_LIBS})

# opencv4
find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(Rasterizer  ${OpenCV_LIBS})
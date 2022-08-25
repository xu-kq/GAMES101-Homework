cmake_minimum_required(VERSION 3.10)

# vcpkg 
# CMAKE_TOOLCHAIN_FILE should be set before project defined
find_path(vcpkg_path vcpkg.exe)
set(CMAKE_TOOLCHAIN_FILE ${vcpkg_path}/scripts/buildsystems/vcpkg.cmake)

project(BezierCurve)
set(CMAKE_CXX_STANDARD 14)
add_executable(BezierCurve main.cpp)

# opencv4
find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(BezierCurve ${OpenCV_LIBS})
#message(STATUS  "OpenCV_LIBS = ${OpenCV_LIBS}")


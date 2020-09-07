cmake_minimum_required(VERSION 3.10)
project(opencv_test)

set(CMAKE_CXX_STANDARD 11)

## The /usr/local/Cellar path is by default included
include_directories(/usr/local/include/opencv4)
link_directories(/usr/local/lib/)
#target_link_directories(opencv_test PUBLIC /usr/local/Cellar/opencv/4.4.0_1/lib/)

find_package(OpenCV REQUIRED)

add_executable(color_calibration_test src/main.cpp)
target_link_libraries(color_calibration_test opencv_core opencv_mcc opencv_highgui opencv_imgcodecs)

#There is no OpenCV cmake files in the path(it's in opencv's source code repo)
#ocv_define_module(opencv_test opencv_core)

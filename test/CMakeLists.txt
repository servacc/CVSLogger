cmake_minimum_required(VERSION 3.16)

include(CTest)

if(NOT BUILD_TESTING)
    return()
endif()

enable_testing()

project(CVSLogger_test)

find_package(PkgConfig REQUIRED)
pkg_search_module(gtest_main REQUIRED IMPORTED_TARGET gtest_main)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE
    factory_test.cpp)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    PkgConfig::gtest_main
    cvs::CVSLogger)

add_executable(${PROJECT_NAME}_opencv)

target_sources(${PROJECT_NAME}_opencv
  PRIVATE
    opencv_test.cpp
    opencv_cfg_test.cpp)

target_link_libraries(${PROJECT_NAME}_opencv
  PUBLIC
    PkgConfig::gtest_main
    cvs::CVSLogger_opencv)

target_compile_definitions(${PROJECT_NAME}_opencv
  PRIVATE
    TEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/logs")

add_test(Cfg_test ${PROJECT_NAME})
add_test(OpenCV_test ${PROJECT_NAME}_opencv)

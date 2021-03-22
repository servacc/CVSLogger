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
        factory_test.cpp
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:opencv_test.cpp>
    )

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        PkgConfig::gtest_main
        cvs::CVSLogger_opencv
    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
    )

add_test(LoggerTest ${PROJECT_NAME})

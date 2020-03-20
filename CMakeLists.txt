cmake_minimum_required(VERSION 3.16)

include(cmake/GitHelpers.cmake)
include(cmake/BuildNumber.cmake)

init_build_number()
git_description(GIT_DESCRIPTOR GIT_TAG GIT_COUNT)

message(STATUS "${GIT_DESCRIPTOR}")
message(STATUS "${GIT_TAG}")
message(STATUS "${GIT_COUNT}")

project(cvslogger
    VERSION 0.0.${GIT_COUNT}${BUILD_NUMBER}
    DESCRIPTION "CVS logger library (${REV_ID})")

option(CVSLOGGER_TESTS "" OFF)
option(CVSLOGGER_OPENCV_IMG "" OFF)

include(GenerateExportHeader)

set(SPDLOG_BUILD_SHARED ON  CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/spdlog)

if(CVSLOGGER_OPENCV_IMG)
    find_package(OpenCV REQUIRED)
endif()

file( GLOB SRCS
        src/cvs/logger/logger.h
        src/cvs/logger/logger.cpp
    )

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRCS}
    )

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME
        ${CMAKE_CURRENT_BINARY_DIR}/export/cvs/logger/${PROJECT_NAME}_export.h
    )

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        spdlog::spdlog
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:opencv_core>
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:opencv_imgcodecs>
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
#        $<INSTALL_INTERFACE:${INCLUDE_LOCATION}>
    )

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:CVS_LOGGER_OPENCV_ENABLED>
    )


target_compile_features(${PROJECT_NAME}
    PUBLIC
        cxx_std_20
    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        POSITION_INDEPENDENT_CODE ON
    )

if(CVSLOGGER_TESTS)
    add_subdirectory(thirdparty/googletest)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    add_subdirectory(test)
endif()

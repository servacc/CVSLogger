cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)
include(cmake/GitHelpers.cmake)

git_description(GIT_DESCRIPTOR GIT_TAG GIT_COUNT)

message(WARNING "TODO: Installing developer components has not been tested.")

project(cvslogger
    VERSION 1.1.${GIT_COUNT}
    DESCRIPTION "CVS logger library (${REV_ID})")

option(CVSLOGGER_SHARED "" ON)
option(CVSLOGGER_TESTS "" OFF)
option(CVSLOGGER_OPENCV_IMG "" OFF)

option(CVSLOGGER_INSTALL "" OFF)
option(CVSLOGGER_DEV_INSTALL "" OFF)

option(CVSLOGGER_ENABLE_STD_BY_DEFAULT "Enable stdout sink by default" OFF)
option(CVSLOGGER_ENABLE_SYSD_BY_DEFAULT "Enable systemd sink by default" ON)

include(GenerateExportHeader)

if(CVSLOGGER_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(thirdparty/spdlog)

find_package(PkgConfig)
pkg_check_modules(PC_SYSTEMD QUIET libsystemd)

if(CVSLOGGER_OPENCV_IMG)
    find_package(OpenCV REQUIRED)
endif()

file( GLOB SRCS
        include/cvs/logger/logging.h
        include/cvs/logger/ilogger.h
        include/cvs/logger/loggerfactory.h
        include/cvs/logger/configtypes.h
        include/cvs/logger/defaultfactory.h

        src/defaultfactory.cpp
        src/configtypes.cpp
        src/loggerfactory.cpp
    )

add_library(${PROJECT_NAME} ${LIB_TYPE})

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
        systemd
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    PRIVATE
        ${PC_SYSTEMD_INCLUDE_DIRS}
    )

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:CVS_LOGGER_OPENCV_ENABLED>
        $<$<BOOL:${CVSLOGGER_ENABLE_STD_BY_DEFAULT}>:CVSLOGGER_STD_ENABLED>
        $<$<BOOL:${CVSLOGGER_ENABLE_SYSD_BY_DEFAULT}>:CVSLOGGER_SYSD_ENABLED>

    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        POSITION_INDEPENDENT_CODE ON
    )

if(CVSLOGGER_TESTS)
    if(NOT TARGET gtest)
        add_subdirectory(thirdparty/googletest)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    endif()

    add_subdirectory(test)
endif()

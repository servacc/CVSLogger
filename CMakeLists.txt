cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)
include(cmake/GitHelpers.cmake)

git_description(GIT_DESCRIPTOR GIT_TAG GIT_COUNT)

message(WARNING "TODO: Installing developer components has not been tested.")

project(cvslogger
    VERSION 2.0.${GIT_COUNT}
    DESCRIPTION "CVS logger library (${REV_ID})")

option(CVSLOGGER_SHARED "" ON)
option(CVSLOGGER_TESTS "" OFF)
option(CVSLOGGER_OPENCV_IMG "" OFF)

option(CVSLOGGER_INSTALL "" OFF)
option(CVSLOGGER_DEV_INSTALL "" OFF)

option(CVSLOGGER_ENABLE_STD_BY_DEFAULT "Enable stdout sink by default" OFF)
option(CVSLOGGER_ENABLE_SYSD_BY_DEFAULT "Enable systemd sink by default" ON)

include(GenerateExportHeader)

if (NOT SPDLOG_FMT_EXTERNAL OR NOT SPDLOG_FMT_EXTERNAL_HO)
    message(NOTICE "Set SPDLOG_FMT_EXTERNAL to ON. "
        "FPSLogger is not compatible with internal fmt in spdlog version 1.8.2 and older.")
    set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "")
endif()

if(NOT TARGET fmt::fmt)
    find_package(fmt REQUIRED)
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
        include/cvs/logger/tools/fpslogger.h

        src/default/defaultfactory.h

        src/default/defaultfactory.cpp
        src/tools/fpslogger.cpp
        src/configtypes.cpp
        src/loggerfactory.cpp
        src/ilogger.cpp
    )

if(CVSLOGGER_SHARED)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_library(${PROJECT_NAME} STATIC)
endif()

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
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        add_subdirectory(thirdparty/googletest)
    endif()

    add_subdirectory(test)
endif()

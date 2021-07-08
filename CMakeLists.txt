cmake_minimum_required(VERSION 3.16)

find_package(CVSCommonCMakeHelpers REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${CVSCOMMON_MODULE_PATH}")

include(CVSCommon)

project(CVSLogger CVS_PROJECT
                  VERSION 3.0.0
                  DESCRIPTION "CVS logger library")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

option(CVSLOGGER_SHARED "" ON)
option(CVSLOGGER_OPENCV_IMG "" ON)

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

find_package(CVSCommon REQUIRED)
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(systemd REQUIRED IMPORTED_TARGET libsystemd)

if(CVSLOGGER_SHARED)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()

cvs_add_to_alias(${PROJECT_NAME})
cvs_library_version(${PROJECT_NAME})
cvs_create_version_file(NAME ${PROJECT_NAME}
                        MAJOR ${PROJECT_VERSION_MAJOR}
                        MINOR ${PROJECT_VERSION_MINOR}
                        PATCH ${PROJECT_VERSION_PATCH}
                        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generate/cvs/logger")

generate_export_header(${PROJECT_NAME}
  EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/export/cvs/logger/${PROJECT_NAME_LOWER}export.hpp)

target_sources(${PROJECT_NAME}
  PRIVATE
    include/cvs/logger/logging.hpp
    include/cvs/logger/loggable.hpp
    include/cvs/logger/tools/fpslogger.hpp
    include/cvs/logger/argumentpreprocessor.hpp

    src/tools/fpslogger.cpp
    src/logging.cpp)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    spdlog::spdlog
    PkgConfig::systemd
    cvs::CVSCommon
    Boost::boost)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generate>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    SPDLOG_FMT_EXTERNAL
    $<$<BOOL:${CVSLOGGER_ENABLE_STD_BY_DEFAULT}>:CVSLOGGER_STD_ENABLED>
    $<$<BOOL:${CVSLOGGER_ENABLE_SYSD_BY_DEFAULT}>:CVSLOGGER_SYSD_ENABLED>)

add_library(${PROJECT_NAME}_opencv SHARED)
add_library(cvs::${PROJECT_NAME}_opencv ALIAS ${PROJECT_NAME}_opencv)

target_sources(${PROJECT_NAME}_opencv
  PRIVATE
    include/cvs/logger/opencvhelper.hpp

    src/opencvhelper.cpp)

target_link_libraries(${PROJECT_NAME}_opencv
  PUBLIC
    cvs::CVSLogger
    opencv_imgcodecs opencv_core)

target_compile_definitions(${PROJECT_NAME}_opencv
  PUBLIC
    CVS_LOGGER_OPENCV_ENABLED)

add_subdirectory(test)

include(CVSInstall)

cvs_install(TARGETS_BIN ${PROJECT_NAME}
  TARGETS_DEV ${PROJECT_NAME}_opencv
  FILES_DEV
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/cvs/logger
    FILES include/cvs/logger/logging.hpp
      include/cvs/logger/loggable.hpp
      include/cvs/logger/tools/fpslogger.hpp
      include/cvs/logger/argumentpreprocessor.hpp
      ${CMAKE_CURRENT_BINARY_DIR}/export/cvs/logger/${PROJECT_NAME_LOWER}export.hpp
      ${CMAKE_CURRENT_BINARY_DIR}/generate/cvs/logger/version.hpp
      include/cvs/logger/opencvhelper.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/cvs/logger/tools
    FILES include/cvs/logger/tools/fpslogger.hpp)

cvs_package()

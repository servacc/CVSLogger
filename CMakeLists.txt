cmake_minimum_required(VERSION 3.16)

find_package(CVSCommon REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${CVSCOMMON_MODULE_PATH}")

include(GNUInstallDirs)
include(CVSGitHelpers)
include(CVSCommon)

cvs_git_description(GIT_DESCRIPTOR GIT_TAG GIT_COUNT)

project(CVSLogger
  VERSION 3.0.${GIT_COUNT}
  DESCRIPTION "CVS logger library (${GIT_DESCRIPTOR})"
  HOMEPAGE_URL ${CVS_HOMEPAGE_URL})

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

find_package(Boost)
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(systemd REQUIRED IMPORTED_TARGET libsystemd)

if(CVSLOGGER_OPENCV_IMG)
  find_package(OpenCV REQUIRED)
endif()

if(CVSLOGGER_SHARED)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()
add_library(cvs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

cvs_library_version()

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
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    $<$<BOOL:${CVSLOGGER_ENABLE_STD_BY_DEFAULT}>:CVSLOGGER_STD_ENABLED>
    $<$<BOOL:${CVSLOGGER_ENABLE_SYSD_BY_DEFAULT}>:CVSLOGGER_SYSD_ENABLED>)

add_library(${PROJECT_NAME}_opencv STATIC)
add_library(cvs::${PROJECT_NAME}_opencv ALIAS ${PROJECT_NAME}_opencv)

target_sources(${PROJECT_NAME}_opencv
  PRIVATE
    include/cvs/logger/opencvhelper.hpp

    src/opencvhelper.cpp)

target_link_libraries(${PROJECT_NAME}_opencv
  PUBLIC
    cvs::CVSLogger
    "$<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:opencv_imgcodecs;opencv_core>")

target_include_directories(${PROJECT_NAME}_opencv
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>)

target_compile_definitions(${PROJECT_NAME}_opencv
  PUBLIC
    $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:CVS_LOGGER_OPENCV_ENABLED>)

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
      include/cvs/logger/opencvhelper.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/cvs/logger/tools
    FILES include/cvs/logger/tools/fpslogger.hpp)

cvs_package()

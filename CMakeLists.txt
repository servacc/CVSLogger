cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)
include(cmake/GitHelpers.cmake)

git_description(GIT_DESCRIPTOR GIT_TAG GIT_COUNT)

project(CVSLogger
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

find_package(spdlog)
find_package(PkgConfig)
pkg_check_modules(systemd QUIET IMPORTED_TARGET libsystemd)

if(CVSLOGGER_OPENCV_IMG)
    find_package(OpenCV REQUIRED)
endif()


if(CVSLOGGER_SHARED)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_library(${PROJECT_NAME} STATIC)
endif()

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME
        ${CMAKE_CURRENT_BINARY_DIR}/export/cvs/logger/${PROJECT_NAME}_export.hpp
    )

target_sources(${PROJECT_NAME}
      PRIVATE
        include/cvs/logger/logging.hpp
        include/cvs/logger/ilogger.hpp
        include/cvs/logger/loggerfactory.hpp
        include/cvs/logger/configtypes.hpp
        include/cvs/logger/tools/fpslogger.hpp

        src/default/defaultfactory.hpp

        src/default/defaultfactory.cpp
        src/tools/fpslogger.cpp
        src/configtypes.cpp
        src/loggerfactory.cpp
        src/ilogger.cpp
    )


target_link_libraries(${PROJECT_NAME}
    PUBLIC
        spdlog::spdlog
        PkgConfig::systemd
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:opencv_core>
        $<$<BOOL:${CVSLOGGER_OPENCV_IMG}>:opencv_imgcodecs>
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
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
    add_subdirectory(test)
endif()

include(install.cmake)

cvsinstall(TARGETS ${PROJECT_NAME}
           VERSION ${PROJECT_VERSION}
           NAME ${PROJECT_NAME}
           HEADERS
             SUBFOLDER cvs/logger
             FILES include/cvs/logger/logging.hpp
                   include/cvs/logger/ilogger.hpp
                   include/cvs/logger/loggerfactory.hpp
                   include/cvs/logger/configtypes.hpp
                   ${CMAKE_CURRENT_BINARY_DIR}/export/cvs/logger/${PROJECT_NAME}_export.hpp
             SUBFOLDER cvs/logger/tools
             FILES include/cvs/logger/tools/fpslogger.hpp
             )

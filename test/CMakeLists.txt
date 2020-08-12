cmake_minimum_required(VERSION 3.16)

project(cvslogger_test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
#        test.cpp
        factory_test.cpp
    )

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        gtest_main
        cvslogger
    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
    )

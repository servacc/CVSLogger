include(CMakeParseArguments)
include(CMakePackageConfigHelpers)

macro(cvsinstall)
  set(options)
  set(oneValueArgs VERSION NAME CONFIG)
  set(multiValueArgs TARGETS HEADERS)
  cmake_parse_arguments(CVSINSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  list(LENGTH CVSINSTALL_HEADERS HEDERS_LENGTH)
  if(HEDERS_LENGTH GREATER 0)
    set(CVSINSTALL_ENABLE_DEV ON)
  else()
    set(CVSINSTALL_ENABLE_DEV OFF)
  endif()

  while(HEDERS_LENGTH GREATER 0)
    set(HEADER_INSTALL_PATH "${CMAKE_INSTALL_INCLUDEDIR}/${CVSINSTALL_NAME}")

    list(GET CVSINSTALL_HEADERS 0 TAG)
    if(TAG STREQUAL "SUBFOLDER")
      math(EXPR INDEX "${INDEX} + 1" OUTPUT_FORMAT DECIMAL)
      list(GET CVSINSTALL_HEADERS 1 SUBFOLDER)
      list(SUBLIST CVSINSTALL_HEADERS 1 -1 CVSINSTALL_HEADERS)
      set(HEADER_INSTALL_PATH "${HEADER_INSTALL_PATH}/${SUBFOLDER}")
    endif()

    list(FIND CVSINSTALL_HEADERS "FILES" INDEX)
    list(GET CVSINSTALL_HEADERS ${INDEX} TAG)
    if(TAG STREQUAL "FILES")
      math(EXPR INDEX "${INDEX} + 1" OUTPUT_FORMAT DECIMAL)
    else()
      message(FATAL_ERROR "Expect \"FILES\" tag.")
    endif()

    list(FIND CVSINSTALL_HEADERS "SUBFOLDER" END)
    if(END GREATER -1)
      math(EXPR LENGHT "${END} - ${INDEX}")
    else()
      set(LENGHT ${END})
    endif()
    list(SUBLIST CVSINSTALL_HEADERS ${INDEX} ${LENGHT} FILES_LIST)

    install(FILES ${FILES_LIST}
      DESTINATION ${HEADER_INSTALL_PATH}
      COMPONENT dev)

    if(END EQUAL -1)
      break()
    endif()
    list(SUBLIST CVSINSTALL_HEADERS ${END} -1 CVSINSTALL_HEADERS)
    list(LENGTH CVSINSTALL_HEADERS HEDERS_LENGTH)
  endwhile()

  install(TARGETS ${CVSINSTALL_TARGETS}
    EXPORT  ${CVSINSTALL_NAME}Targets
    RUNTIME
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT   bin
    LIBRARY
      DESTINATION          ${CMAKE_INSTALL_LIBDIR}
      COMPONENT            bin
    ARCHIVE
      DESTINATION          ${CMAKE_INSTALL_LIBDIR}
      COMPONENT            dev
     )

  if(CVSINSTALL_ENABLE_DEV)
    install(EXPORT ${CVSINSTALL_NAME}Targets
      FILE         ${CVSINSTALL_NAME}Targets.cmake
      NAMESPACE    cvs::
      DESTINATION  ${CMAKE_INSTALL_LIBDIR}/cmake/${CVSINSTALL_NAME}
      COMPONENT    dev)

    configure_package_config_file(${CVSINSTALL_CONFIG}
      ${CMAKE_CURRENT_BINARY_DIR}/cvsinstall/${CVSINSTALL_NAME}Config.cmake
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CVSINSTALL_NAME}
      NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)

    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/cvsinstall/${CVSINSTALL_NAME}ConfigVersion.cmake
      VERSION ${CVSINSTALL_VERSION}
      COMPATIBILITY SameMajorVersion)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cvsinstall/${CVSINSTALL_NAME}Config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/cvsinstall/${CVSINSTALL_NAME}ConfigVersion.cmake
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CVSINSTALL_NAME}
      COMPONENT   dev)
  endif()
endmacro()

macro(cvspackage)
  set(options)
  set(oneValueArgs PACKAGE_NAME)
  set(multiValueArgs)
  cmake_parse_arguments(CVSINSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(CPACK_GENERATOR "DEB")

  set(CPACK_DEBIAN_BIN_PACKAGE_NAME "${CVSINSTALL_PACKAGE_NAME}")

  set(CPACK_COMPONENT_UNSPECIFIED_GROUP "bin")
  set(CPACK_COMPONENT_BIN_GROUP "bin")
  set(CPACK_COMPONENT_DEV_GROUP "dev")

  set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "${CVSINSTALL_PACKAGE_NAME} (= ${PROJECT_VERSION})")

  set(CPACK_PACKAGE_CONTACT "CVS")

  set(CPACK_DEB_COMPONENT_INSTALL ON)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

  include(CPack)
endmacro()

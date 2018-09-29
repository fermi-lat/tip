cmake_minimum_required(VERSION 3.11)
include(GNUInstallDirs)
include(FetchContent)

project(tip
  VERSION 0.0.0
  LANGUAGES CXX
  )

###############################################################################
#
# External dependencies managed through fetchcontent
#
###############################################################################

fetchcontent_declare(cfitsio
  GIT_REPOSITORY https://github.com/HEASARC/cfitsio.git
  GIT_TAG master
)

fetchcontent_getproperties(cfitsio)
if(NOT cfitsio_POPULATED)
  fetchcontent_populate(cfitsio)
  add_subdirectory(${cfitsio_SOURCE_DIR} ${cfitsio_BINARY_DIR})
endif()


###############################################################################


###############################################################################
#
# Tip targets
#
###############################################################################

add_library(tip STATIC
  src/FileSummary.cxx
  src/FitsFileManager.cxx
  src/FitsHeader.cxx
  src/FitsPrimProps.cxx
  src/FitsTable.cxx
  src/FitsTipFile.cxx
  src/Header.cxx
  src/IFileSvc.cxx
  src/KeyRecord.cxx
  src/LinearInterp.cxx
  src/TipException.cxx
  src/TipFile.cxx
)

add_library(ST::tip ALIAS tip)

target_include_directories(tip
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(tip
  PRIVATE
    cfitsio::cfitsio
)


set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/tip)

install(DIRECTORY tip DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS tip
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

install(TARGETS tip
  EXPORT tipTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

install(EXPORT tipTargets
  NAMESPACE    ST::
  DESTINATION  ${INSTALL_CONFIGDIR}
  )

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/tipConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/tipConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/tipConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/tipConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/tipConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT tipTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/tipTargets.cmake
  NAMESPACE ST::
)

#Register package in user's package registry
export(PACKAGE tip)


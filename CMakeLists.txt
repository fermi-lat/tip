####################################
# Create target and set properties #
####################################
add_library(
  tip STATIC
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

target_compile_definitions(tip PUBLIC BUILD_WITHOUT_ROOT)
target_include_directories(
  tip PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>
)

target_link_libraries(tip PUBLIC facilities cfitsio::cfitsio)


if(FERMI_BUILD_ROOT)
  target_sources(tip PRIVATE src/RootTable.cxx)
  target_link_libraries(tip PUBLIC ROOT::Core)
endif()


add_executable(
  test_tip
  src/test/test_tip_main.cxx
  src/test/TestColumn.cxx
  src/test/TestExtensionData.cxx
  src/test/TestFileManager.cxx
  src/test/TestFileSummary.cxx
  src/test/TestFilter.cxx
  src/test/TestHarness.cxx
  src/test/TestImage.cxx
  src/test/TestInterpolation.cxx
  src/test/TestKeyRecord.cxx
  src/test/TestTable.cxx
)

target_include_directories(
  test_tip PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:>
)
target_link_libraries(test_tip tip)

###############################################################
# Installation
###############################################################
install(DIRECTORY tip DESTINATION ${FERMI_INSTALL_INCLUDEDIR})
install(DIRECTORY data/ DESTINATION ${FERMI_INSTALL_REFDATADIR}/tip)

install(
  TARGETS tip test_tip
  EXPORT fermiTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

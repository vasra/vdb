add_library(libvdb process.cpp pipe.cpp)
add_library(vdb::libvdb ALIAS libvdb)

set_target_properties(libvdb PROPERTIES OUTPUT_NAME vdb)

target_include_directories(
  libvdb
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

include(GNUInstallDirs)
install(
  TARGETS libvdb
  EXPORT vdb-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT vdb-targets
  FILE vdb-config.cmake
  NAMESPACE vdb::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vdb)

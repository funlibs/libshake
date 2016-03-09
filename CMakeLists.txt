cmake_minimum_required (VERSION 2.6)
project (SNDO)


# build PortAudio
if (CMAKE_SYSTEM MATCHES "Windows")

    add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)
    add_subdirectory(deps/portaudio)

else()

    # we are on linux/unix/mac use configure script
    include (ExternalProject)
    ExternalProject_Add(portaudio
        SOURCE_DIR          ${CMAKE_SOURCE_DIR}/deps/portaudio
        PREFIX              ${CMAKE_BINARY_DIR}/portaudio
        CONFIGURE_COMMAND   <SOURCE_DIR>/configure
        BUILD_COMMAND       make
        INSTALL_COMMAND     echo Skipping install step for portaudio)

endif(CMAKE_SYSTEM MATCHES "Windows")


# build SNDO
include_directories (.)
add_library (sndo sndo.c sndo.h)
 
add_dependencies (sndo portaudio)


# doc
find_package (Doxygen)

if (DOXYGEN_FOUND)

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

  add_custom_target (sndo_doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)

endif (DOXYGEN_FOUND)

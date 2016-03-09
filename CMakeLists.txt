cmake_minimum_required (VERSION 2.6)
project (SNDO)



# build PortAudio
set (EXTRA_LIBS "")
set (PA_INCLUDE_DIR "")
set (PA_LIBRARY_DIR "")

if (CMAKE_SYSTEM MATCHES "Windows")

    add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)
    add_subdirectory(deps/portaudio)
    set(PA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/deps/portaudio/include/)

else()
    # we are on linux/unix/apple use configure script

    if (APPLE)

        find_library(CORE_AUDIO_LIBRARY CoreAudio)
        find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
        find_library(AUDIO_UNIT_LIBRARY AudioUnit)
        find_library(CARBON_LIBRARY Carbon)
        set(EXTRA_LIBS
            ${CORE_AUDIO_LIBRARY}
            ${AUDIO_TOOLBOX_LIBRARY}
            ${AUDIO_UNIT_LIBRARY}
            ${CARBON_LIBRARY})

    else()

        find_library(ALSA REQUIRED)
        set(EXTRA_LIBS -lrt -lm -lasound -ljack -pthread)

    endif()

    include (ExternalProject)
    ExternalProject_Add(ext_portaudio
        SOURCE_DIR          ${CMAKE_SOURCE_DIR}/deps/portaudio
        PREFIX              ${CMAKE_BINARY_DIR}/portaudio
        CONFIGURE_COMMAND   <SOURCE_DIR>/configure
        BUILD_COMMAND       make
        INSTALL_COMMAND     echo Skipping install step for portaudio)
    ExternalProject_Get_Property(ext_portaudio BINARY_DIR)
    ExternalProject_Get_Property(ext_portaudio SOURCE_DIR)
    set(PA_LIBRARY_DIR "${BINARY_DIR}/lib/.libs")
    set(PA_INCLUDE_DIR "${SOURCE_DIR}/include")
    add_library (portaudio STATIC IMPORTED)
    set_property(
        TARGET portaudio
        PROPERTY IMPORTED_LOCATION "${PA_LIBRARY_DIR}/libportaudio.a")

endif(CMAKE_SYSTEM MATCHES "Windows")



# build SNDO
include_directories (. ${PA_INCLUDE_DIR})
add_library (sndo sndo.c sndo.h)
add_dependencies (sndo portaudio)


# tests
add_executable (sndo_test
    sndo_test.c
    sndo.c
    sndo.h)
target_link_libraries(sndo_test portaudio ${EXTRA_LIBS})
add_dependencies(sndo_test sndo)

enable_testing()
add_test (NAME SineWaveOutput   COMMAND sndo_test)


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

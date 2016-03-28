cmake_minimum_required (VERSION 2.6)
project (SHAKE)

#==============================================================================
# SHAKE library
#==============================================================================
add_library (shake STATIC shake.c shake.h)
include_directories (.)


#==============================================================================
# EXAMPLE
#==============================================================================
add_executable (shake_example
    shake_example.c
    shake.c
    shake.h)
target_link_libraries(shake_example shake)
add_dependencies(shake_example shake)


#==============================================================================
# DOC
#==============================================================================
find_package (Doxygen)

if (DOXYGEN_FOUND)

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

  add_custom_target (shake_doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)

endif (DOXYGEN_FOUND)
cmake_minimum_required (VERSION 2.6)
project (TESTEXT)



#==============================================================================
# EXTERN
#==============================================================================
include (ExternalProject)
set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR})

if(WIN32)

    set(PA_GENERATOR ${CMAKE_GENERATOR})
    set(PA_CONFIGURE "")
    set(PA_BUILD     "")

elseif(UNIX)

    set(PA_GENERATOR "")
    set(PA_CONFIGURE <SOURCE_DIR>/configure)
    set(PA_BUILD     make)

    if(APPLE)
        find_library(CORE_AUDIO_LIBRARY CoreAudio)
        find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
        find_library(AUDIO_UNIT_LIBRARY AudioUnit)
        find_library(CARBON_LIBRARY Carbon)
    else() # LINUX/UNIX
        find_library(ALSA REQUIRED)
    endif(APPLE)

endif(WIN32)


#
# LIBWAVE
#
ExternalProject_Add(ext_wave
    GIT_REPOSITORY  https://github.com/ssbx/libwave.git
    GIT_TAG         master
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD    0
    LOG_CONFIGURE   0
    LOG_BUILD       0)
add_dependencies(shake ext_wave)
include_directories(${CMAKE_BINARY_DIR}/Source/ext_wave)
set(WAVE_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}wave${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(shake
    ${CMAKE_BINARY_DIR}/Build/ext_wave/${WAVE_LIB})

#
# PORTAUDIO
#
ExternalProject_Add(ext_portaudio
    GIT_REPOSITORY  https://github.com/ssbx/portaudio.git
    GIT_TAG         master
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    CONFIGURE_COMMAND ${PA_CONFIGURE}
    BUILD_COMMAND   ${PA_BUILD}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD    0
    LOG_CONFIGURE   0
    LOG_BUILD       0)
add_dependencies(shake ext_portaudio)
include_directories(${CMAKE_BINARY_DIR}/Source/ext_portaudio/include)
set(PA_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}portaudio${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(shake
    ${CMAKE_BINARY_DIR}/Build/ext_portaudio/lib/.libs/${PA_LIB})


#==============================================================================
# EXTRA LIBS
#==============================================================================
if (UNIX)

    if(APPLE)

        target_link_libraries(shake
            ${CORE_AUDIO_LIBRARY}
            ${AUDIO_TOOLBOX_LIBRARY}
            ${AUDIO_UNIT_LIBRARY}
            ${CARBON_LIBRARY})

    else() # LINUX/UNIX

        target_link_libraries(shake
            -lrt 
            -lm 
            -lasound 
            -ljack 
            -pthread)

    endif(APPLE)

endif(UNIX)


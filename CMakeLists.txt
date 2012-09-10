# CMakeLists for Goliath

#############
# Setup stuff
#############

# minimum cmake version 2.6 required by the scripts to get Git information
cmake_minimum_required (VERSION 2.6)

# define the project
project (goliath)

# find OpenCV.  This, for some totally bizarre reason, needs to come *before*
# the CMAKE_MODULE_PATH is created for FFTW.
find_package (OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIR})
set (LIBS ${LIBS} ${OpenCV_LIBS})

# we need to use our cmake files
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" )

# Setup the default install prefix
# This gets set to the binary directory upon first configuring.
# If the user changes the prefix, but leaves the flag OFF, then it will remain as the user specified.
# If the user wants to reset the prefix to the default (i.e. the binary directory), then the flag should be set ON.
if (NOT DEFINED SET_INSTALL_PREFIX_TO_DEFAULT)
    set (SET_INSTALL_PREFIX_TO_DEFAULT ON)
endif (NOT DEFINED SET_INSTALL_PREFIX_TO_DEFAULT)
if (SET_INSTALL_PREFIX_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR} CACHE PATH "Install prefix" FORCE)
    set (SET_INSTALL_PREFIX_TO_DEFAULT OFF CACHE BOOL "Reset default install path when when configuring" FORCE)
endif (SET_INSTALL_PREFIX_TO_DEFAULT)

# install subdirectories
set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Install directory for headers")
set (LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Install directory for libraries")
set (BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Install directory for binaries")

# build shared libraries
set (BUILD_SHARED_LIBS ON)

# find Protocol Buffers
find_package (Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
set (LIBS ${LIBS} ${PROTOBUF_LIBRARIES})

# find FFTW
find_package (FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIR})
set (LIBS ${LIBS} ${FFTW_LIBRARY})

# let's be fancy!
set( CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}")

# monarch pls
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/monarch/Install/include")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/monarch/Install/lib")

##############
# Build things
##############

# Lists of header and source files

#set (MONARCH_PROTOBUF_HEADER_FILES
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/MonarchHeader.proto
# )

set (GOLIATH_HEADER_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_const.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_signal.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_xfrmr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_env.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_tfr_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_candidate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_color.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_discrim.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_tfr_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_util.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glth_geom.hpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set (GOLIATH_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_signal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_xfrmr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_io.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_tfr_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_candidate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_discrim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_tfr_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_geom.cpp
)

# Generate the Protocol Buffers cpp file
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#protobuf_generate_cpp (GENERATED_PROTO_SOURCE_FILES GENERATED_PROTO_HEADER_FILES ${MONARCH_PROTOBUF_HEADER_FILES})
#install (FILES ${GENERATED_PROTO_HEADER_FILES}  DESTINATION ${INCLUDE_INSTALL_DIR})


#install (FILES ${MONARCH_HEADER_FILES}  DESTINATION ${INCLUDE_INSTALL_DIR})
#install (TARGETS monarch  DESTINATION ${LIB_INSTALL_DIR})

# Build the test executable
add_executable (glth ${CMAKE_CURRENT_SOURCE_DIR}/src/glth.cpp ${GOLIATH_SOURCE_FILES})
target_link_libraries(glth MonarchCore ${LIBS})
add_executable (glth_test ${CMAKE_CURRENT_SOURCE_DIR}/src/glth_test.cpp ${GOLIATH_SOURCE_FILES})
target_link_libraries(glth_test MonarchCore ${LIBS})

install (TARGETS DESTINATION ${BIN_INSTALL_DIR})

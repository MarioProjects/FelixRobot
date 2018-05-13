# set minimum cmake version required and cmake policy
cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(CXX)

# Name the project
project(iatros-htr-online)
set(PROJECT_FULLNAME "iatros-htr-online")

# Add customized cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake.modules/")


# Use doxygen to generate documentation
include(Documentation)
if(BUILD_DOCUMENTATION AND DOXYGEN_FOUND)
  configure_file(${CMAKE_SOURCE_DIR}/doxyfile.cmake ${CMAKE_SOURCE_DIR}/doc/doxyfile)
  add_custom_target(doc ${DOXYGEN} ${PROJECT_SOURCE_DIR}/doc/doxyfile)
endif(BUILD_DOCUMENTATION AND DOXYGEN_FOUND)

# Include package generation
include(Package)

include(FindCustomPackage)

# Build type options
#default build. should equal to release build 

include(CommonCompileOptions)

#http://www.vtk.org/Wiki/CMake_HowToDoPlatformChecks
set(CMAKE_REQUIRED_INCLUDES $ENV{HOME}/local/include)
include(CheckIncludeFiles)
include(CheckFunctionExists)

check_include_files(fftw3.h HAVE_FFTW3_H)
if(NOT HAVE_FFTW3_H)
  message(SEND_ERROR "FFTW3 devel library missing")
endif(NOT HAVE_FFTW3_H)

#add extra include directories for generated source files
include_directories(${CMAKE_BINARY_DIR})

# add support for hyades and aldebaran compilation
# libraries must be instaled in $HOME/local
include_directories($ENV{HOME}/local/include)
link_directories($ENV{HOME}/local/lib)

# add package creation
include(CPack)

include(LibTools)

# enable testing
#include(CTest)
enable_testing()
message(STATUS "Enabling testing")

# Add build info variables and version
include(BuildInfo)

# Process the subdirectories
add_subdirectory(src)

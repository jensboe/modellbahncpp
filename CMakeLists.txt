cmake_minimum_required(VERSION 4.0)
include(cmake/ToolChain.cmake)

project(modelbahn CXX C)
add_library(project_options INTERFACE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ENABLE_CPPCHECK OFF)
set(ENABLE_MODM_CHECK_EXCLUDE ON)


add_library(project_warnings INTERFACE)
include(cmake/Cache.cmake)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# include(cmake/Doxygen.cmake)
# enable_doxygen()
include(cmake/StaticAnalyzers.cmake)

option(ENABLE_IPO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" OFF)
if(ENABLE_IPO)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_subdirectory(modm)
add_subdirectory(modellbahn)
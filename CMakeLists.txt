# Author:  Johannes de Fine Licht (johannes.definelicht@inf.ethz.ch)
# Created: October 2016
cmake_minimum_required(VERSION 2.8.12)
project(SDAccelExample CXX)

# This CMake file was only written for configuration purposes: it does not
# provide an installation independent on the source directory, but merely
# allows modifying source files without changing the original files.

set(EXAMPLE_PLATFORM_NAME "virtex7" CACHE STRING
    "Name of platform in Vivado HLS." ) 
set(EXAMPLE_DSA_STRING "xilinx:adm-pcie-7v3:1ddr:2.1" CACHE STRING 
    "DSA string of target platform.")
set(EXAMPLE_ENTRY "Kernel" CACHE STRING "Name of top level function")

set(EXAMPLE_DATA_TYPE "float" CACHE STRING "Data type")
set(EXAMPLE_DATA_WIDTH 4 CACHE STRING "Width of data path in bytes")

set(EXAMPLE_ADDS_PER_STAGE 1 CACHE STRING "")
set(EXAMPLE_MULTS_PER_STAGE 1 CACHE STRING "")
set(EXAMPLE_COMPUTE_STAGES 8 CACHE STRING "")

set(EXAMPLE_ELEMENTS 8192 CACHE STRING "Number of elements to stream" )
set(EXAMPLE_ITERATIONS 128 CACHE STRING "Default number of iterations to run")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(SDAccel REQUIRED)
if (NOT SDACCEL_FOUND)
  message(FATAL_ERROR "SDAccel not found.")
endif()

find_package(VivadoHLS REQUIRED)
if (NOT VIVADO_HLS_FOUND)
  message(FATAL_ERROR "Vivado HLS not found.")
endif()
include_directories(${CMAKE_BINARY_DIR} include
                    ${SDACCEL_INCLUDE_DIRS} ${VIVADO_HLS_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

configure_file(${CMAKE_SOURCE_DIR}/Kernel.h.in Kernel.h)
configure_file(${CMAKE_SOURCE_DIR}/script/Synthesis.tcl.in Synthesis.tcl)
configure_file(${CMAKE_SOURCE_DIR}/script/CSim.tcl.in CSim.tcl)
configure_file(${CMAKE_SOURCE_DIR}/script/CoSim.tcl.in CoSim.tcl)

set(EXAMPLE_HLS_DEPENDS
    ${CMAKE_SOURCE_DIR}/Kernel.cpp 
    ${CMAKE_SOURCE_DIR}/Kernel.h.in)

add_executable(RunHardware ${CMAKE_SOURCE_DIR}/RunHardware.cpp)
target_link_libraries(RunHardware ${SDACCEL_LIBRARIES}) 

add_custom_target(synthesis
  COMMAND ${VIVADO_HLS_BINARY} -f Synthesis.tcl
  DEPENDS ${EXAMPLE_HLS_DEPENDS})

add_custom_target(csim
  COMMAND ${VIVADO_HLS_BINARY} -f CSim.tcl
  DEPENDS ${EXAMPLE_HLS_DEPENDS})

add_custom_target(cosim
  COMMAND ${VIVADO_HLS_BINARY} -f CoSim.tcl
  DEPENDS ${EXAMPLE_HLS_DEPENDS})

add_custom_target(hw
  COMMAND XILINX_SDACCEL=${SDACCEL_ROOT_DIR} ${SDACCEL_XOCC} -t hw
  --kernel ${EXAMPLE_ENTRY} --xdevice ${EXAMPLE_DSA_STRING}
  -I${CMAKE_BINARY_DIR} -I${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/Kernel.cpp
  -o ${EXAMPLE_ENTRY}.xclbin
  DEPENDS ${EXAMPLE_HLS_DEPENDS})

add_custom_target(run_hw
  COMMAND LD_PRELOAD=${SDACCEL_LIBRARY_DIR}/libxilinxopencl.so ./RunHardware)

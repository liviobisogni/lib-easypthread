#******************************************************************************
#**************** CMAKELISTS.TXT - Compiling 'myptask' library ****************
#****************             Author: Livio Bisogni            ****************
#****************         © 2021 REAL-TIME INDUSTRY Inc.       ****************
#******************************************************************************

cmake_minimum_required(VERSION 3.1)

# Project name
project(myptask)

# Directory of files
set(SDIR "./src")

set(SOURCE_FILES
	${SDIR}/myptask.c
	${SDIR}/myptime.c
)

set(HEADER_FILES
	${SDIR}/myptask.h
	${SDIR}/myptime.h
)

# Create a library called "myptask" which includes the source files
add_library(myptask "STATIC" "${SOURCE_FILES}")
  
# Export
export(TARGETS myptask FILE "./myptaskConfig.cmake")
export(PACKAGE myptask)

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
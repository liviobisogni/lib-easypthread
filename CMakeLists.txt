#******************************************************************************
#*********** CMAKELISTS.TXT - Compiling 'easy_pthread_task' library ***********
#***********                  Author: Livio Bisogni                 ***********
#******************************************************************************

cmake_minimum_required(VERSION 3.1)

# Project name
project(easy_pthread_task)

# Directory of files
set(SDIR "./src")

set(SOURCE_FILES
	${SDIR}/easy_pthread_task.c
	${SDIR}/easy_pthread_time.c
)

set(HEADER_FILES
	${SDIR}/easy_pthread_task.h
	${SDIR}/easy_pthread_time.h
)

# Create a library called "easy_pthread_task" which includes the source files
add_library(easy_pthread_task "STATIC" "${SOURCE_FILES}")
  
# Export
export(TARGETS easy_pthread_task FILE "./easypthreadConfig.cmake")
export(PACKAGE easy_pthread_task)

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
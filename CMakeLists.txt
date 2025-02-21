###########################################################################
#   Project: bufr2synop                                                   #
#                                                                         #
#   Copyright (C) 2016-2024 by Guillermo Ballester Valor                  #
#   gbvalor@ogimet.com                                                    #
#                                                                         #
#   GNU General Public License v2 or above                                #
###########################################################################
#
# CMadkeLists.txt for root directory for project bufr2synop
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.10)

# Next line is to set version 
cmake_policy(SET CMP0048 NEW)

# Establecemos la version 
project (bufr2synop LANGUAGES C VERSION 0.32.0)

#Definitions needed to generate config.h
SET(PACKAGE_NAME "${PROJECT_NAME}")
SET(PACKAGE_VERSION "${PROJECT_VERSION}")
SET(VERSION "${PROJECT_VERSION}")
SET(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
SET(VERSION_MINOR ${PROJECT_VERSION_MINOR})
SET(VERSION_PATCH ${PROJECT_VERSION_PATCH})
SET(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
SET(BUILD_USING_CMAKE 1)

message (STATUS "** Configuring package ${PROJECT_NAME}. Version ${PROJECT_VERSION} **")

# current source and build directory at compile time will be in include directories 
SET (CMAKE_INCLUDE_CURRENT_DIR ON)

# Usual system include directories
list (APPEND all_includes "/usr/include" "/usr/local/include")

# Usual system library directories
list (APPEND dir_libs "/usr/lib" "/usr/local/lib") 

# Check if default compiler is Clang based 
if (CMAKE_C_COMPILER_ID MATCHES "Clang")
  # using Clang compiler
  set(CMAKE_C_FLAGS "-O2 -Wall -W -Wextra -g")
  add_link_options("-Wl,-no_warn_duplicate_libraries")
  set( compiler_id_message "*****  Using Clang compiler ****")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  # using GNU compiler
  set(CMAKE_C_FLAGS "-O2 -Wall -W -Wextra -g -Wno-stringop-truncation")
  if (APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-overflow" )
  else()
    list (APPEND dir_libs "/usr/lib64" "/usr/local/lib64") 
  endif()
  set( compiler_id_message "******  Using GNU compiler *****")
endif()


# Check if need to try Intel icc complier
IF (DEFINED TRY_ICC)
  if ("${TRY_ICC}" STREQUAL "ON")
    MESSAGE(STATUS "***** Trying to use Intel compiler c/c++ **") 
    # If not found icc then will use gcc
    find_program(ICC_PROGRAM icc)
    IF(ICC_PROGRAM)
      MESSAGE(STATUS "** Using Intel c/c++ compiler **")
      set(CMAKE_C_COMPILER "icc")
      set(CMAKE_C_FLAGS "-O2 -xHOST -Wall")
      set(USING_ICC 1)
    else (ICC_PROGRAM)
      MESSAGE(STATUS "*** Intel compiler not found ***")
      MESSAGE(STATUS "${compiler_id_message}")
      set(USING_GCC 1) 
    ENDIF(ICC_PROGRAM)
  ELSE()  
    MESSAGE(STATUS "${compiler_id_message}")
    set(USING_GCC 1) 
  ENDIF()
ELSE()
  MESSAGE(STATUS "${compiler_id_message}")
  set(USING_GCC 1) 
ENDIF()

# Summary of compiler options
MESSAGE(STATUS "  CMAKE_C_COMPILER:   ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "  CMAKE_C_FLAGS:      ${CMAKE_C_FLAGS}")
MESSAGE(STATUS "**************************************")

# Uncomment following line if we want a verbose output at build time
#SET(CMAKE_VERBOSE_MAKEFILE ON)


#####################################################################################
###### Some checks about libraries that msut be installed.            ###############
###### Some results willhwlp to build config.h file                   ###############
#####################################################################################

# Needed to check some symbols in libraries
include(CheckSymbolExists)

set(CMAKE_REQUIRED_LIBRARIES "m")
check_symbol_exists(floor "math.h" HAVE_FLOOR)
check_symbol_exists(atan2 "math.h" HAVE_ATAN2)

FIND_FILE(STRING_H "string.h" PATHS ${all_includes})
IF (STRING_H)
 SET(HAVE_STRING_H 1)
 check_symbol_exists(strstr "string.h" HAVE_STRSTR)
ELSE (STRING_H)
  MESSAGE(FATAL_ERROR "string.h not found!")
ENDIF(STRING_H)

FIND_FILE(STDLIB_H "stdlib.h" PATHS ${all_includes})
IF (STDLIB_H)
 SET(HAVE_STDLIB_H 1)
 check_symbol_exists(getenv "stdlib.h" HAVE_GETENV)
ELSE (STDLIB_H)
  MESSAGE(FATAL_ERROR "stdlib.h not found!")
ENDIF(STDLIB_H)

FIND_FILE(UNISTD_H "unistd.h" PATHS ${all_includes})
IF (UNISTD_H)
 SET(HAVE_UNISTD_H 1)
 check_symbol_exists(read "unistd.h" HAVE_READ)
ELSE (UNISTD_H)
  MESSAGE(FATAL_ERROR "unistd.h not found!")
ENDIF(UNISTD_H)

FIND_FILE(SYS_TIME_H "sys/time.h" PATHS ${all_includes})
IF (SYS_TIME_H)
 SET(HAVE_SYS_TIME_H 1)
 check_symbol_exists(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)
ENDIF(SYS_TIME_H)

FIND_FILE(SYS_STAT_H "sys/stat.h" PATHS ${all_includes})
IF (SYS_STAT_H)
 SET(HAVE_SYS_STAT_H 1)
 check_symbol_exists(stat "sys/stat.h" HAVE_STAT)
ENDIF(SYS_STAT_H)


if (BUFRDC)
  FIND_LIBRARY(BUFR bufr PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64)
  IF (BUFR)
    MESSAGE("-- ECMWF BUFR library found!")
  ELSE (BUFR)
    MESSAGE(FATAL_ERROR "Cannot found ECMWF BUFR library. Please install it")
  ENDIF(BUFR)
endif (BUFRDC)

######################################################################
###### create config.h fromconfig.h.cmake.in      ####################
######################################################################
configure_file (cmake_config.h.in config.h)

######################################################################
###### create uninstall target, cmake does not include it ############
######################################################################
# make uninstall
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstall files with the aid of install_manifest.txt"
    COMMAND xargs rm -vf < ${CMAKE_BINARY_DIR}/install_manifest.txt || echo "Nothing to uninstall according to install_manifest.txt!"
)

# Esto debe ponerse antes que add_subdir() para que se propague 
# a los subdirectorios
include(GNUInstallDirs)
include (CPack)

add_subdirectory(src)
add_subdirectory(share)
add_subdirectory(examples)



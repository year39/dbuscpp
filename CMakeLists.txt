cmake_minimum_required(VERSION 3.8)

project(dbuscpp
  VERSION 1.0
  DESCRIPTION "dbus c++ library based on sdbus")

option(build_type_debug "Build Debug" ON)
option(generate_compile_commands "Generate compile_commands.json" ON)

if(${build_type_debug})
  set(CMAKE_BUILD_TYPE Debug)
  message("==> Build Type: Debug")
  message("==> Debug Symbols Generated")
else()
  set(CMAKE_BUILD_TYPE Release)
  message("==> Build Type: Release")
endif()

# generate compile_commands.json
if(${generate_compile_commands})
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

##################################################
# sd-bus library
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD "libsystemd" REQUIRED) #install libsystemd-dev
if(NOT SYSTEMD_FOUND)
  message(FATAL_ERROR "ERROR: Systemd not found! Make sure to install libsystemd-dev")
else()
  message(STATUS "Systemd: v${SYSTEMD_VERSION} found")
  set(LIBSYSTEMD_VERSION ${SYSTEMD_VERSION}) # can be accessed from the source file #if LIBSYSTEMD_VERSION >= 237
endif(NOT SYSTEMD_FOUND)

##################################################
# boost library
FIND_PACKAGE( Boost 1.73.0 REQUIRED )
IF (Boost_FOUND)
  message(STATUS "Boost v${Boost_VERSION} found")
  INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR})
endif()

##################################################
set(dbuscpp_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/connection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/message.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/reply.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/signal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/signal_group.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/manager.cpp)

set(dbuscpp_private_hdrs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/internal.h)

set(dbuscpp_public_hdrs
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/connection.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/manager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/reply.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/signal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/signal_group.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/dbuscpp/dbuscpp.h)

set(dbuscpp_all_srcs ${dbuscpp_srcs} ${dbuscpp_private_hdrs} ${dbuscpp_public_hdrs})
set(library_name ${PROJECT_NAME})

# generate .so
add_library(${library_name} SHARED ${dbuscpp_srcs})
#set_property(TARGET ${library_name} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})

target_include_directories(${library_name}
  PUBLIC
    $<INSTALL_INTERFACE:include> # this is for the user's of this package to locate the headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${library_name} PROPERTIES
  LINKER_LANGUAGE CXX
  PUBLIC_HEADER "${dbuscpp_public_hdrs}" # won't work with the double quotes
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  OUTPUT_NAME ${library_name}
)

target_link_libraries(${library_name} PUBLIC
  ${SYSTEMD_LIBRARIES}
  ${Boost_LIBRARIES})

# Leave CMAKE_CXX_FLAGS Alone
target_compile_options(${library_name} PRIVATE -Wall -Wextra)
target_compile_features(${library_name} PRIVATE cxx_std_17)

##################################################
# install targets
include(GNUInstallDirs)

# NOTE:
# find_package(lib CONFIG). If you don't do this,
# CMake will first search for a module package (e.g. FindMyLibrary.cmake).
# If it can't find one, *then* it does config package mode.
# You should always use the newer config package of cmake

set(library_target_export "${library_name}-targets")
install(
  TARGETS 	${library_name}
  EXPORT    ${library_target_export}
  ARCHIVE  	DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  	DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  	DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${library_name}
)

install(FILES ${dbuscpp_public_hdrs} DESTINATION DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${library_name})

# install the .cmake of the target
install(
  EXPORT ${library_target_export}
  FILE   dbuscppConfig.cmake # find_package(dbuscpp CONFIG)
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}
  NAMESPACE dbuscpp:: # target_link_libraries(target dbuscpp::dbuscpp)
)

##################################################
# ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/dbuscppConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/dbuscppConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}
)

##################################################
# register package in user's package registry
export(PACKAGE ${PROJECT_NAME})

##################################################
# example on how to use this library
#
# find_package(dbuscpp 1.0 CONFIG)
# if(${dbuscpp_FOUND})
#   message(STATUS "dbuscpp v${dbuscpp_VERSION} found")
#   message(STATUS "dbuscpp directory: ${dbuscpp_DIR}")
# else()
#   message(FATAL_ERROR "dbuscpp not found")
# endif()
# add_executable(temp src/message.cpp)
# target_link_libraries(temp dbuscpp::dbuscpp)

##################################################
##################################################

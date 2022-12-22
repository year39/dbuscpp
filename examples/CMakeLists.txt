cmake_minimum_required(VERSION 3.8)

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
  message(STATUS "Systemd: ${SYSTEMD_VERSION}")
  set(LIBSYSTEMD_VERSION ${SYSTEMD_VERSION}) # can be accessed from the source file #if LIBSYSTEMD_VERSION >= 237
endif(NOT SYSTEMD_FOUND)

find_package (Threads)

##################################################
# temporary for debug
# add_executable(dbus_test src/dbus.cpp src/signal_handler.cpp)
# target_link_libraries(dbus_test
#   ${SYSTEMD_LIBRARIES}
#   Threads::Threads)

# target_compile_options(dbus_test PRIVATE -Wall)
# target_compile_features(dbus_test PRIVATE cxx_std_17)

# boost library
FIND_PACKAGE( Boost 1.73.0 REQUIRED )
IF (Boost_FOUND)
  message(STATUS "Boost v${Boost_VERSION} found")
  INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR})
endif()

find_package(dbuscpp 1.0 CONFIG REQUIRED)
if(${dbuscpp_FOUND})
  message(STATUS "dbuscpp v${dbuscpp_VERSION} found")
  message(STATUS "dbuscpp directory: ${dbuscpp_DIR}")
else()
  message(FATAL_ERROR "dbuscpp not found")
endif()

add_executable(ex_method src/ex_method.cpp)
target_link_libraries(ex_method dbuscpp::dbuscpp)
target_compile_options(ex_method PRIVATE -Wall -Wextra)
target_compile_features(ex_method PRIVATE cxx_std_17)

add_executable(ex_signals src/ex_signals.cpp)
target_link_libraries(ex_signals dbuscpp::dbuscpp ${Boost_LIBRARIES})
target_compile_options(ex_signals PRIVATE -Wall -Wextra)
target_compile_features(ex_signals PRIVATE cxx_std_17)

add_executable(ex_copy src/ex_copy.cpp)
target_link_libraries(ex_copy dbuscpp::dbuscpp)
target_compile_options(ex_copy PRIVATE -Wall -Wextra)
target_compile_features(ex_copy PRIVATE cxx_std_17)

add_executable(ex_bluez_signals src/ex_bluez_signals.cpp)
target_link_libraries(ex_bluez_signals dbuscpp::dbuscpp)
target_compile_options(ex_bluez_signals PRIVATE -Wall -Wextra)
target_compile_features(ex_bluez_signals PRIVATE cxx_std_17)

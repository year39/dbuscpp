#pragma once
#include <errno.h>
#include <stdexcept>
#include <string>
#include <systemd/sd-bus.h>

namespace dbus {

inline void THROW_EXCEPTION_IF( bool condition, std::string message ) {
  if ( condition )
    throw std::runtime_error( message );
}

inline void THROW_EXCEPTION_IF( bool condition, std::string message, int code ) {
  if ( condition ) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    ::sd_bus_error_set_errno( &err, code );
    std::string throw_message = message + " (" + err.message + ")";
    ::sd_bus_error_free( &err );
    THROW_EXCEPTION_IF( true, message );
  }
}

inline void THROW_EXCEPTION_IF( bool condition, std::string message, sd_bus_error *err ) {
  if ( condition ) {
    std::string s;
    if ( err ) {
      s = err->message;
      sd_bus_error_free( err );
      message += " (" + s + ")";
    }
    THROW_EXCEPTION_IF( true, message );
  }
}

}  // namespace dbus

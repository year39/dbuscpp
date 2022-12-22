
#include "dbuscpp/connection.h"
#include "internal.h"
#include <string>
#include <systemd/sd-bus.h>

using namespace dbus;

Connection::Connection( int connectionType ) {
  int r = 0;
  if ( connectionType == ConnectionType::REUSE_SYSTEM_DBUS )
    r = ::sd_bus_default_system( (sd_bus **)&bus );
  else if ( connectionType == ConnectionType::NEW_SYSTEM_DBUS )
    r = ::sd_bus_open_system( (sd_bus **)&bus );
  else
    THROW_EXCEPTION_IF( true, "Unknown connectionType (" + std::to_string( connectionType ) + ")" );
  THROW_EXCEPTION_IF( r < 0, "Failed to create system bus connection", -r );
}

Connection::Connection( const Connection &other ) {
  std::lock_guard<std::mutex> lock( mutex );
  if ( bus )
    bus = ::sd_bus_unref( (sd_bus *)bus );
  bus = ::sd_bus_ref( (sd_bus *)other.bus );
}

Connection::~Connection() {
  bus = ::sd_bus_flush_close_unref( (sd_bus *)bus );
}

Connection &Connection::operator=( const Connection &rhs ) {
  if ( this == &rhs )
    return *this;

  std::lock_guard<std::mutex> lock( mutex );
  bus = ::sd_bus_flush_close_unref( (sd_bus *)bus );
  bus = ::sd_bus_ref( (sd_bus *)rhs.bus );
  return *this;
}

std::string Connection::address() {
  if ( !ready() )
    return std::string {};

  const char *address;
  int r = ::sd_bus_get_address( (sd_bus *)bus, &address );

  THROW_EXCEPTION_IF( r < 0, "Failed to obtain bus address", -r );

  return std::string { address };
}

bool Connection::open() {
  int r = ::sd_bus_is_open( (sd_bus *)bus );
  return r >= 0;
}

bool Connection::ready() {
  int r = ::sd_bus_is_ready( (sd_bus *)bus );
  return r >= 0;
}

std::chrono::microseconds Connection::timeout() {
  uint64_t usec;
  int r = ::sd_bus_get_timeout( (sd_bus *)bus, &usec );
  THROW_EXCEPTION_IF( r < 0, "Failed to read bus timeout", -r );
  return std::chrono::microseconds { usec };
}

void *Connection::borrowBusObject() {
  THROW_EXCEPTION_IF( !ready(), "Failed to borrow bus object, connection is not established" );
  return bus;
}

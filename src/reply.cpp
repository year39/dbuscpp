#include "dbuscpp/reply.h"
#include "internal.h"
#include <iostream>
#include <string>
#include <systemd/sd-bus.h>

using namespace dbus;

Reply::Reply() {}

Reply::Reply( void *message ) {
  THROW_EXCEPTION_IF( !message, "Failed to create Message with null pointer" );
  msg = message;
}

Reply::Reply( const Reply &other ) {
  if ( msg )
    msg = ::sd_bus_message_unref( (sd_bus_message *)msg );
  msg = ::sd_bus_message_ref( (sd_bus_message *)other.msg );
}

Reply &Reply::operator=( const Reply &rhs ) {
  if ( this == &rhs )
    return *this;
  std::lock_guard<std::mutex> lock( mutex );

  if ( msg )
    msg = ::sd_bus_message_unref( (sd_bus_message *)msg );
  msg = ::sd_bus_message_ref( (sd_bus_message *)rhs.msg );
  return *this;
}

Reply::~Reply() {
  msg = ::sd_bus_message_unref( (sd_bus_message *)msg );
}

bool Reply::empty() {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_is_empty( (sd_bus_message *)msg );
  return r == 0;
}

int Reply::type() {
  uint8_t msg_type = 0;
  std::lock_guard<std::mutex> lock( mutex );
  ::sd_bus_message_get_type( (sd_bus_message *)msg, &msg_type );
  return msg_type;
}

bool Reply::signatureValid( std::string signature ) {
  // TODO make this check smarter
  bool is_valid = false;
  for ( const char c : signature ) {
    if ( c == static_cast<char>( DATA_TYPE::BYTE ) ||
         c == static_cast<char>( DATA_TYPE::BOOLEAN ) ||
         c == static_cast<char>( DATA_TYPE::INT16 ) ||
         c == static_cast<char>( DATA_TYPE::UINT16 ) ||
         c == static_cast<char>( DATA_TYPE::INT32 ) ||
         c == static_cast<char>( DATA_TYPE::UINT32 ) ||
         c == static_cast<char>( DATA_TYPE::INT64 ) ||
         c == static_cast<char>( DATA_TYPE::UINT64 ) ||
         c == static_cast<char>( DATA_TYPE::DOUBLE ) ||
         c == static_cast<char>( DATA_TYPE::STRING ) ||
         c == static_cast<char>( DATA_TYPE::OBJECT_PATH ) ||
         c == static_cast<char>( DATA_TYPE::SIGNATURE ) ||
         c == static_cast<char>( DATA_TYPE::UNIX_FD ) ||
         c == static_cast<char>( DATA_TYPE::ARRAY ) ||
         c == static_cast<char>( DATA_TYPE::VARIANT ) ||
         c == static_cast<char>( DATA_TYPE::STRUCT_BEGIN ) ||
         c == static_cast<char>( DATA_TYPE::STRUCT_END ) ||
         c == static_cast<char>( DATA_TYPE::DICT_ENTRY_BEGIN ) ||
         c == static_cast<char>( DATA_TYPE::DICT_ENTRY_END ) )
      is_valid = true;
    else
      return false;  // no point of continuing the loop
  }
  return is_valid;
}

bool Reply::hasSignature( std::string signature ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_has_signature(
    (sd_bus_message *)msg, signature.c_str() );  // returns 1 if match
  return r == 1;
}

char Reply::signatureType() {
  char sig_type = static_cast<char>( DATA_TYPE::EMPTY );
  const char *sig_contents;

  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_peek_type( (sd_bus_message *)msg, &sig_type, &sig_contents );
  if ( r < 0 )
    sig_type = DATA_TYPE::EMPTY;

  return sig_type;
}

std::string Reply::signatureContents() {
  char sig_type = 0;
  const char *sig_contents;

  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_peek_type( (sd_bus_message *)msg, &sig_type, &sig_contents );

  if ( r >= 0 && sig_contents )
    return std::string { sig_contents };
  return std::string {};
}

void Reply::enterContainer( char type, char content ) {
  std::string contents = { content };
  enterContainer( type, contents );
}

void Reply::enterContainer( char type, std::string contents ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_enter_container( (sd_bus_message *)msg, type, contents.c_str() );
  THROW_EXCEPTION_IF( r <= 0, "Failed to enter container", -r );
}

void Reply::enterContainer() {
  char type = signatureType();
  std::string contents = signatureContents();
  enterContainer( type, contents.c_str() );
}

bool Reply::enterContainerIf( char type, std::string contents ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_enter_container( (sd_bus_message *)msg, type, contents.c_str() );
  return r > 0;
}

void Reply::skip( std::string types ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_skip( (sd_bus_message *)msg, types.c_str() );

  THROW_EXCEPTION_IF( r <= 0, "Failed to skip container", -r );
}
void Reply::exitContainer() {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_exit_container( (sd_bus_message *)msg );
  THROW_EXCEPTION_IF( r < 0, "Failed to exit container", -r );
}

std::string Reply::sender() {
  const char *sender = nullptr;
  sender = ::sd_bus_message_get_sender( (sd_bus_message *)msg );
  if ( sender )
    return std::string { sender };
  return std::string {};
}

std::string Reply::service() {
  const char *service = nullptr;
  service = ::sd_bus_message_get_destination( (sd_bus_message *)msg );
  if ( service )
    return std::string { service };
  return std::string {};
}

std::string Reply::path() {
  const char *object = nullptr;
  object = ::sd_bus_message_get_path( (sd_bus_message *)msg );
  if ( object )
    return std::string { object };
  return std::string {};
}

std::string Reply::interface() {
  const char *interface = nullptr;
  interface = ::sd_bus_message_get_interface( (sd_bus_message *)msg );
  if ( interface )
    return std::string { interface };
  return std::string {};
}

std::string Reply::member() {
  const char *member = nullptr;
  member = ::sd_bus_message_get_member( (sd_bus_message *)msg );
  if ( member )
    return std::string { member };
  return std::string {};
}

void Reply::read( bool &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int p = 0;
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_BOOLEAN, &p );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
  value = static_cast<bool>( p );
}

void Reply::read( int16_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_INT16, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( int32_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_INT32, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( int64_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_INT64, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( uint8_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_BYTE, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( uint16_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_UINT16, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( uint32_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_UINT32, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( uint64_t &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_UINT64, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( double &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_DOUBLE, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );
}

void Reply::read( std::string &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  char *p;
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_STRING, &p );

  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );

  value = std::string( p );
}

void Reply::read( ObjectPath &value ) {
  std::lock_guard<std::mutex> lock( mutex );
  char *p;
  int r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_OBJECT_PATH, &p );

  THROW_EXCEPTION_IF( r < 0, "Failed to read message value", -r );

  value = std::string( p );
}

void Reply::read( std::vector<ObjectPath> &value ) {
  char *p;
  int r = 0;

  enterContainer( DATA_TYPE::ARRAY, DATA_TYPE::OBJECT_PATH );

  std::lock_guard<std::mutex> lock( mutex );
  while ( ( r = ::sd_bus_message_read_basic(
              (sd_bus_message *)msg, SD_BUS_TYPE_OBJECT_PATH, &p ) ) > 0 ) {
    value.push_back( ObjectPath { std::string { p } } );
  }
  ::sd_bus_message_exit_container( (sd_bus_message *)msg );
}

void Reply::read( std::vector<std::string> &value ) {
  char *p;
  int r = 0;

  enterContainer( DATA_TYPE::ARRAY, "s" );

  std::lock_guard<std::mutex> lock( mutex );
  while (
    ( r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_STRING, &p ) ) > 0 ) {
    value.push_back( std::string { p } );
  }
  ::sd_bus_message_exit_container( (sd_bus_message *)msg );
}

void Reply::read( std::vector<std::byte> &value ) {
  uint8_t p = 0;
  std::byte byte;
  int r = 0;

  enterContainer( DATA_TYPE::ARRAY, "y" );

  std::lock_guard<std::mutex> lock( mutex );
  while ( ( r = ::sd_bus_message_read_basic( (sd_bus_message *)msg, SD_BUS_TYPE_BYTE, &p ) ) > 0 ) {
    byte = static_cast<std::byte>( p );
    value.push_back( byte );
  }
  ::sd_bus_message_exit_container( (sd_bus_message *)msg );
}

void *Reply::borrowBusMessage() {
  THROW_EXCEPTION_IF( !msg, "Attempt to aqcuire a null message pointer" );
  return msg;
}

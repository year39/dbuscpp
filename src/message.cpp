
#include "dbuscpp/message.h"
#include "internal.h"
#include <iostream>
#include <string>
#include <systemd/sd-bus.h>

using namespace dbus;

Message::Message() {}

Message::Message( void *message ) {
  THROW_EXCEPTION_IF( !message, "Failed to create Message with null pointer" );
  msg = message;
}

Message::Message( const Message &other ) {
  if ( msg )
    msg = ::sd_bus_message_unref( (sd_bus_message *)msg );
  msg = ::sd_bus_message_ref( (sd_bus_message *)other.msg );
}

Message &Message::operator=( const Message &rhs ) {
  if ( this == &rhs )
    return *this;
  std::lock_guard<std::mutex> lock( mutex );

  if ( msg )
    msg = ::sd_bus_message_unref( (sd_bus_message *)msg );
  msg = ::sd_bus_message_ref( (sd_bus_message *)rhs.msg );
  return *this;
}

Message::~Message() {
  msg = ::sd_bus_message_unref( (sd_bus_message *)msg );
}

bool Message::empty() {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_is_empty( (sd_bus_message *)msg );
  return r == 0;
}

int Message::type() {
  uint8_t msg_type = 0;
  std::lock_guard<std::mutex> lock( mutex );
  ::sd_bus_message_get_type( (sd_bus_message *)msg, &msg_type );
  return msg_type;
}

bool Message::signatureValid( std::string signature ) {
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

bool Message::hasSignature( std::string signature ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_has_signature(
    (sd_bus_message *)msg, signature.c_str() );  // returns 1 if match
  return r == 1;
}

char Message::signatureType() {
  char sig_type = static_cast<char>( DATA_TYPE::EMPTY );
  const char *sig_contents;

  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_peek_type( (sd_bus_message *)msg, &sig_type, &sig_contents );
  if ( r < 0 )
    sig_type = DATA_TYPE::EMPTY;

  return sig_type;
}

std::string Message::signatureContents() {
  char sig_type = 0;
  const char *sig_contents;

  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_peek_type( (sd_bus_message *)msg, &sig_type, &sig_contents );

  if ( r >= 0 && sig_contents )
    return std::string { sig_contents };
  return std::string {};
}

void Message::openContainer( char type, char content ) {
  std::string contents = { content };
  openContainer( type, contents );
}

void Message::openContainer( char type, std::string contents ) {
  int r = ::sd_bus_message_open_container( (sd_bus_message *)msg, type, contents.c_str() );
  THROW_EXCEPTION_IF( r < 0, "Failed to open container", -r );
}

void Message::closeContainer() {
  int r = ::sd_bus_message_close_container( (sd_bus_message *)msg );
  THROW_EXCEPTION_IF( r < 0, "Failed to close container", -r );
}

std::string Message::sender() {
  const char *sender = nullptr;
  sender = ::sd_bus_message_get_sender( (sd_bus_message *)msg );
  if ( sender )
    return std::string { sender };
  return std::string {};
}

std::string Message::service() {
  const char *service = nullptr;
  service = ::sd_bus_message_get_destination( (sd_bus_message *)msg );
  if ( service )
    return std::string { service };
  return std::string {};
}

std::string Message::path() {
  const char *object = nullptr;
  object = ::sd_bus_message_get_path( (sd_bus_message *)msg );
  if ( object )
    return std::string { object };
  return std::string {};
}

std::string Message::interface() {
  const char *interface = nullptr;
  interface = ::sd_bus_message_get_interface( (sd_bus_message *)msg );
  if ( interface )
    return std::string { interface };
  return std::string {};
}

std::string Message::member() {
  const char *member = nullptr;
  member = ::sd_bus_message_get_member( (sd_bus_message *)msg );
  if ( member )
    return std::string { member };
  return std::string {};
}

void Message::write( bool value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int temp = value;
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_BOOLEAN, &temp );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( int16_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_INT16, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( int32_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_INT32, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( int64_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_INT64, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( uint8_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_BYTE, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( uint16_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_UINT16, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( uint32_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_UINT32, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( uint64_t value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_UINT64, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( double value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_DOUBLE, &value );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( std::string value ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_STRING, value.c_str() );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void Message::write( std::vector<std::byte> value ) {
  std::lock_guard<std::mutex> lock( mutex );
  uint8_t p = 0;
  int r = 0;
  openContainer( DATA_TYPE::ARRAY, DATA_TYPE::BYTE );

  for ( auto &i : value ) {
    p = static_cast<uint8_t>( i );
    r = ::sd_bus_message_append_basic( (sd_bus_message *)msg, SD_BUS_TYPE_BYTE, &p );
    THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
  }
  closeContainer();
}

void Message::write( ObjectPath objectPath ) {
  std::lock_guard<std::mutex> lock( mutex );
  int r = ::sd_bus_message_append_basic(
    (sd_bus_message *)msg, SD_BUS_TYPE_OBJECT_PATH, objectPath.c_str() );
  THROW_EXCEPTION_IF( r < 0, "Failed to write message value", -r );
}

void *Message::borrowBusMessage() {
  THROW_EXCEPTION_IF( !msg, "Attempt to aqcuire a null message pointer" );
  return msg;
}

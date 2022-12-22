#pragma once
#include "dbuscpp/common.h"
#include <cstddef>
#include <mutex>
#include <string>

namespace dbus {

class Message {
public:
  Message();
  Message( void *message );
  Message( const Message &other );
  ~Message();

  Message &operator=( const Message &rhs );

  int type();
  bool empty();
  bool signatureValid( std::string signature );
  char signatureType();
  bool hasSignature( std::string signature );
  std::string signatureContents();

  void openContainer( char type, char content );
  void openContainer( char type, std::string contents );
  void closeContainer();

  std::string sender();
  std::string service();
  std::string path();
  std::string interface();
  std::string member();

  void write( bool value );
  void write( int16_t value );
  void write( int32_t value );
  void write( int64_t value );
  void write( uint8_t value );
  void write( uint16_t value );
  void write( uint32_t value );
  void write( uint64_t value );
  void write( double value );
  void write( std::string value );
  void write( std::vector<std::byte> value );
  void write( ObjectPath objectPath );

private:
  friend class Manager;
  void *borrowBusMessage();

  void *msg = nullptr;
  std::mutex mutex;
};
}  // namespace dbus

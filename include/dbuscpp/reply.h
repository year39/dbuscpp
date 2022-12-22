#pragma once
#include "dbuscpp/common.h"
#include <cstddef>
#include <mutex>
#include <string>

namespace dbus {

class Reply {
public:
  Reply();
  Reply( void *message );
  Reply( const Reply &other );
  Reply &operator=( const Reply &rhs );
  ~Reply();

  int type();
  bool empty();
  bool signatureValid( std::string signature );
  char signatureType();
  bool hasSignature( std::string signature );
  std::string signatureContents();

  void enterContainer( char type, char content );
  void enterContainer( char type, std::string contents );
  void enterContainer();
  bool enterContainerIf( char type, std::string contents );
  void skip( std::string types );
  void exitContainer();

  std::string sender();
  std::string service();
  std::string path();
  std::string interface();
  std::string member();

  void read( bool &value );
  void read( int16_t &value );
  void read( int32_t &value );
  void read( int64_t &value );
  void read( uint8_t &value );
  void read( uint16_t &value );
  void read( uint32_t &value );
  void read( uint64_t &value );
  void read( double &value );
  void read( std::string &value );
  void read( ObjectPath &value );
  void read( std::vector<ObjectPath> &value );
  void read( std::vector<std::string> &value );
  void read( std::vector<std::byte> &value );

private:
  friend class Manager;
  void *borrowBusMessage();

  void *msg = nullptr;
  std::mutex mutex;
};
}  // namespace dbus

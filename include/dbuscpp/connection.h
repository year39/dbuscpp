#pragma once
#include "dbuscpp/common.h"
#include <chrono>
#include <mutex>

namespace dbus {
enum ConnectionType { REUSE_SYSTEM_DBUS = 0, NEW_SYSTEM_DBUS };
class Connection {
public:
  Connection( int connectionType = ConnectionType::NEW_SYSTEM_DBUS);
  Connection( const Connection &c );
  Connection &operator=( const Connection &rhs );
  ~Connection();

  std::string address();
  bool open();
  bool ready();
  std::chrono::microseconds timeout();

  void *borrowBusObject();

private:
  void *bus = nullptr;
  std::mutex mutex;

};  // class Connection

}  // namespace dbus

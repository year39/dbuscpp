#pragma once
#include "dbuscpp/common.h"
#include "dbuscpp/connection.h"
#include "dbuscpp/manager.h"
#include "dbuscpp/message.h"
#include "dbuscpp/reply.h"
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace dbus {
class Manager {
public:
  Manager();
  Manager( int connectionType );
  Manager( const Manager &m );
  Manager &operator=( const Manager &m );

  Message
  methodCall( std::string service, std::string object, std::string interface, std::string member );

  Reply
  propertyGet( std::string service, std::string object, std::string interface, std::string member );

  Message
  propertySet( std::string service, std::string object, std::string interface, std::string member );

  Reply call( Message m );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    bool &value );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    int16_t &value );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    int32_t &value );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    int64_t &value );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    double &value );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    std::string &value );

  void propertyGetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    ObjectPath &value );

  void propertySetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    bool value );

  void propertySetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    int16_t value );

  void propertySetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    int32_t value );

  void propertySetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    int64_t value );

  void propertySetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    double value );

  void propertySetDirect( std::string service,
    std::string object,
    std::string interface,
    std::string member,
    std::string value );

  // the service must implement DBus org.freedesktop.DBus.ObjectManager
  std::vector<ObjectPath> objects( std::string service );

private:
  std::mutex mutex;
  Connection conn;
};
}  // namespace dbus

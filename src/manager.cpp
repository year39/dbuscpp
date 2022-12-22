#include "dbuscpp/manager.h"
#include "internal.h"
#include <systemd/sd-bus.h>

using namespace dbus;

Manager::Manager() : conn( ConnectionType::NEW_SYSTEM_DBUS ) {}

Manager::Manager( int connectionType ) : conn( connectionType ) {}

Manager::Manager( const Manager &m ) {
  conn = m.conn;
}

Manager &Manager::operator=( const Manager &rhs ) {
  if ( this == &rhs )
    return *this;

  conn = rhs.conn;
  return *this;
}

Message Manager::methodCall( std::string service,
  std::string object,
  std::string interface,
  std::string member ) {
  std::lock_guard<std::mutex> lock( mutex );

  sd_bus_message *msg = nullptr;

  int r = ::sd_bus_message_new_method_call( (sd_bus *)conn.borrowBusObject(),
    (sd_bus_message **)&msg,
    service.c_str(),
    object.c_str(),
    interface.c_str(),
    member.c_str() );
  THROW_EXCEPTION_IF( r < 0, "Failed to create new method call message", -r );

  return Message { msg };
}

Reply Manager::propertyGet( std::string service,
  std::string object,
  std::string interface,
  std::string member ) {
  std::lock_guard<std::mutex> lock( mutex );

  ::sd_bus_error err = SD_BUS_ERROR_NULL;
  sd_bus_message *reply = nullptr;

  int r = ::sd_bus_call_method( (sd_bus *)conn.borrowBusObject(),
    service.c_str(),
    object.c_str(),
    "org.freedesktop.DBus.Properties",
    "Get",
    &err,
    (sd_bus_message **)&reply,
    "ss",
    interface.c_str(),
    member.c_str() );

  THROW_EXCEPTION_IF( r < 0, "Failed to create new method call", &err );

  return Reply { reply };
}

Message Manager::propertySet( std::string service,
  std::string object,
  std::string interface,
  std::string member ) {
  std::lock_guard<std::mutex> lock( mutex );

  sd_bus_message *msg = nullptr;

  int r = ::sd_bus_message_new_method_call( (sd_bus *)conn.borrowBusObject(),
    (sd_bus_message **)&msg,
    service.c_str(),
    object.c_str(),
    "org.freedesktop.DBus.Properties",
    "Set" );

  THROW_EXCEPTION_IF( r < 0, "Failed to create new method call message", -r );

  r = ::sd_bus_message_append( (sd_bus_message *)msg, "ss", interface.c_str(), member.c_str() );

  THROW_EXCEPTION_IF( r < 0, "Failed to create new method call message", -r );

  return Message { msg };
}

Reply Manager::call( Message m ) {
  std::lock_guard<std::mutex> lock( mutex );

  ::sd_bus_error err = SD_BUS_ERROR_NULL;

  sd_bus_message *reply = nullptr;

  int r = ::sd_bus_call( (sd_bus *)conn.borrowBusObject(),
    (sd_bus_message *)m.borrowBusMessage(),
    0,
    &err,
    (sd_bus_message **)&reply );

  THROW_EXCEPTION_IF( r < 0, "Failed to create new method call", &err );

  return Reply { reply };
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  bool &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::BOOLEAN );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  int16_t &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::INT16 );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  int32_t &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::INT32 );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  int64_t &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::INT64 );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  double &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::DOUBLE );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  std::string &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::STRING );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertyGetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  ObjectPath &value ) {
  Reply reply = propertyGet( service, object, interface, member );
  reply.enterContainer( DATA_TYPE::VARIANT, DATA_TYPE::OBJECT_PATH );
  reply.read( value );
  reply.exitContainer();
}

void Manager::propertySetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  bool value ) {
  Message m = propertySet( service, object, interface, member );
  m.openContainer( DATA_TYPE::VARIANT, DATA_TYPE::BOOLEAN );
  m.write( value );
  m.closeContainer();
  call( m );
}

void Manager::propertySetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  int16_t value ) {
  Message m = propertySet( service, object, interface, member );
  m.openContainer( DATA_TYPE::VARIANT, std::string { DATA_TYPE::INT16 } );
  m.write( value );
  m.closeContainer();
  call( m );
}

void Manager::propertySetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  int32_t value ) {
  Message m = propertySet( service, object, interface, member );
  m.openContainer( DATA_TYPE::VARIANT, DATA_TYPE::INT32 );
  m.write( value );
  m.closeContainer();
  call( m );
}

void Manager::propertySetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  int64_t value ) {
  Message m = propertySet( service, object, interface, member );
  m.openContainer( DATA_TYPE::VARIANT, DATA_TYPE::INT64 );
  m.write( value );
  m.closeContainer();
  call( m );
}

void Manager::propertySetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  double value ) {
  Message m = propertySet( service, object, interface, member );
  m.openContainer( DATA_TYPE::VARIANT, DATA_TYPE::DOUBLE );
  m.write( value );
  m.closeContainer();
  call( m );
}

void Manager::propertySetDirect( std::string service,
  std::string object,
  std::string interface,
  std::string member,
  std::string value ) {
  Message m = propertySet( service, object, interface, member );
  m.openContainer( DATA_TYPE::VARIANT, DATA_TYPE::STRING );
  m.write( value );
  m.closeContainer();
  call( m );
}

std::vector<ObjectPath> Manager::objects( std::string service ) {
  std::vector<ObjectPath> objects;
  ObjectPath object;
  Message msg =
    methodCall( service, "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects" );
  Reply reply = call( msg );  // a{oa{sa{sv}}}

  if ( !( reply.signatureType() == DATA_TYPE::ARRAY ) &&
       ( reply.signatureContents() == "{oa{sa{sv}}}" ) )
    return objects;  // something's wrong!

  reply.enterContainer( DATA_TYPE::ARRAY, "{oa{sa{sv}}}" );

  while ( reply.enterContainerIf( DATA_TYPE::DICT_ENTRY, "oa{sa{sv}}" ) ) {
    reply.read( object );
    objects.push_back( object );
    reply.skip( "a{sa{sv}}" );
    reply.exitContainer();
  }

  reply.exitContainer();  // top container
  return objects;
}

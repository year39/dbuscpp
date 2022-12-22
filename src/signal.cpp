#include "dbuscpp/signal.h"
#include "systemd/sd-bus.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace dbus;

Signal::Signal() {
  m_rule = std::string {};
  m_uuid = boost::uuids::random_generator()();
  m_slot = nullptr;
}

Signal::Signal( std::string rule, std::function<void( SignalID )> callback ) {
  m_rule = rule;
  m_callback = callback;
  m_uuid = boost::uuids::random_generator()();
  m_slot = nullptr;
}

Signal::Signal( const Signal& other ) {
  if ( this == &other )
    return;
  if ( m_slot )
    m_slot = ::sd_bus_slot_unref( (sd_bus_slot*)m_slot );
  if ( other.m_slot )
    m_slot = ::sd_bus_slot_ref( (sd_bus_slot*)other.m_slot );
  m_status = other.m_status;
  m_rule = other.m_rule;
  m_callback = other.m_callback;
  m_statusCallback = other.m_statusCallback;
  m_uuid = other.m_uuid;
}

Signal& Signal::operator=( const Signal& rhs ) {
  if ( m_slot )
    m_slot = ::sd_bus_slot_unref( (sd_bus_slot*)m_slot );
  if ( rhs.m_slot )
    m_slot = ::sd_bus_slot_ref( (sd_bus_slot*)rhs.m_slot );
  m_status = rhs.m_status;
  m_rule = rhs.m_rule;
  m_callback = rhs.m_callback;
  m_statusCallback = rhs.m_statusCallback;
  m_uuid = rhs.m_uuid;
  return *this;
}

bool Signal::operator==( const Signal& rhs ) {
  if ( m_uuid == rhs.m_uuid )
    return true;
  return false;
}

Signal::~Signal() {
  if ( m_slot )
    m_slot = ::sd_bus_slot_unref( (sd_bus_slot*)m_slot );
}

void Signal::callback() {
  if ( m_callback )
    m_callback( m_uuid );
}

void Signal::registerCallback( std::function<void( SignalID )> cb ) {
  m_callback = cb;
}

boost::uuids::uuid Signal::uuid() {
  return m_uuid;
}

std::string Signal::rule() {
  return m_rule;
}

void Signal::updateRule( std::string other ) {
  m_rule = other;
};

void Signal::statusCallback() {
  if ( m_statusCallback )
    m_statusCallback( m_uuid );
}

void Signal::registerStatusCallback( std::function<void( SignalID )> cb ) {
  m_statusCallback = cb;
}

void Signal::updateStatus( SignalStatus status ) {
  m_status = status;
  statusCallback();
}

SignalStatus Signal::status() {
  return m_status;
}

void* Signal::slot() {
  return m_slot;
}

void Signal::updateSlot( void* other ) {
  m_slot = other;
}

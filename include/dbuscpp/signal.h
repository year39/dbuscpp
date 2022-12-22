#pragma once
#include "dbuscpp/reply.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <functional>
#include <string>

/* match rule:
 * https://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-routing-match-rules
 */

namespace dbus {

using SignalID = boost::uuids::uuid;

enum SignalStatus { UNDEFINED = 0, ADD_REQUEST, ADDED, MATCH_FAILED, REMOVE_REQUEST, REMOVED };

class Signal {
public:
  Signal();
  Signal( std::string rule, std::function<void( SignalID )> callback );
  Signal( const Signal& other );
  Signal& operator=( const Signal& rhs );
  bool operator==( const Signal& rhs );
  ~Signal();
  std::string rule();
  void updateRule( std::string other );
  SignalID uuid();
  void callback();
  void registerCallback( std::function<void( SignalID )> cb );
  void statusCallback();
  void registerStatusCallback( std::function<void( SignalID )> cb );
  void updateStatus( SignalStatus status );
  SignalStatus status();
  void* slot();
  void updateSlot( void* other );

private:
  SignalID m_uuid;
  void* m_slot = nullptr;
  SignalStatus m_status = SignalStatus::UNDEFINED;
  std::string m_rule;
  std::function<void( SignalID )> m_callback = nullptr;
  std::function<void( SignalID )> m_statusCallback = nullptr;

};  // class Signal

}  // namespace dbus

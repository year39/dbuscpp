
#include <dbuscpp/dbuscpp.h>
#include <iostream>
#include <string>

using namespace dbus;

void callback1( dbus::SignalID uuid ) {
  std::cout << "callback1: " << uuid << "\n";
}

inline void wait_for( int seconds ) {
  std::this_thread::sleep_for( std::chrono::seconds( seconds ) );
}

void statusCallback( dbus::SignalID uuid ) {
  std::cout << "status changed: ";
  if ( SignalGroup::status( uuid ) == SignalStatus::ADDED )
    std::cout << "match added\n";
  if ( SignalGroup::status( uuid ) == SignalStatus::MATCH_FAILED )
    std::cout << "match failed\n";
  if ( SignalGroup::status( uuid ) == SignalStatus::ADD_REQUEST )
    std::cout << "match request\n";
  if ( SignalGroup::status( uuid ) == SignalStatus::REMOVE_REQUEST )
    std::cout << "remove request\n";
  if ( SignalGroup::status( uuid ) == SignalStatus::REMOVED )
    std::cout << "removed\n";
}

int main() {
  auto id1 = SignalGroup::createSignal();
  SignalGroup::matchRule( id1, "type=signal" );
  SignalGroup::signalCallback( id1, callback1 );
  SignalGroup::signalStatusCallback( id1, statusCallback );
  std::cout << "Signal1 ID: " << id1 << "\n";
  SignalGroup::add( id1 );

  SignalGroup::start();
  wait_for( 10 );
  return 0;
}

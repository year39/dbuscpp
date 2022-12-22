#include "dbuscpp/signal_group.h"
#include "dbuscpp/reply.h"
#include "internal.h"
#include <assert.h>
#include <iostream>
#include <sys/poll.h>
#include <systemd/sd-bus.h>

namespace dbus {
int match_callback( sd_bus_message *msg, void *userdata, sd_bus_error *error ) {
  std::ignore = error;
  if ( userdata != NULL && msg != NULL ) {
    Signal *s = static_cast<Signal *>( userdata );
    // check status in case the signal was removed after the callback was triggered
    if ( s->status() == SignalStatus::ADDED ) {
      s->callback();
    }
    return 0;
  }
  return -1;
}
}  // namespace dbus

using namespace dbus;

SignalGroupImp::SignalGroupImp() : loopThread() {}

SignalGroupImp::~SignalGroupImp() {
  stop();
  // unref the match slots, which ends the match
  for ( auto &s : signals ) {
    void *slot = s.slot();
    if ( slot ) {
      slot = ::sd_bus_slot_unref( (sd_bus_slot *)slot );
      s.updateSlot( slot );
    }
    signals.erase( signals.begin(), signals.end() );
  }
}

SignalID SignalGroupImp::createSignal() {
  Signal sig;

  sig.updateSlot( nullptr );
  sig.updateStatus( SignalStatus::UNDEFINED );

  auto uuid = sig.uuid();

  std::lock_guard<std::mutex> lock( mutex );
  signals.push_back( std::move( sig ) );

  return uuid;
}

bool SignalGroupImp::matchRule( SignalID uuid, std::string rule ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid ) {
      s.updateRule( rule );
      return true;
    }
  return false;
}

bool SignalGroupImp::signalCallback( SignalID uuid, std::function<void( SignalID )> callback ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid ) {
      s.registerCallback( callback );
      return true;
    }
  return false;
}

bool SignalGroupImp::signalStatusCallback( SignalID uuid,
  std::function<void( SignalID )> callback ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid ) {
      s.registerStatusCallback( callback );
      return true;
    }
  return false;
}

bool SignalGroupImp::add( SignalID uuid ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid ) {
      if ( s.status() != SignalStatus::ADDED ) {
        s.updateStatus( SignalStatus::ADD_REQUEST );
        signalChanged = true;
      }
      return true;
    }
  return false;
}

bool SignalGroupImp::contains( SignalID uuid ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid )
      return true;
  return false;
}

void SignalGroupImp::remove( SignalID uuid ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid ) {
      s.updateStatus( SignalStatus::REMOVE_REQUEST );
      signalChanged = true;
      break;
    }
}

void SignalGroupImp::start() {
  if ( !loopRunning && !stopRequest ) {
    std::lock_guard<std::mutex> lock( mutex );
    stopRequest = false;
    loopThread = std::thread { &SignalGroupImp::eventLoop, this };
    loopRunning = true;
  }
}

void SignalGroupImp::stop() {
  if ( loopRunning )
    stopRequest = true;
  if ( loopThread.joinable() )
    loopThread.join();
}

SignalStatus SignalGroupImp::status( SignalID uuid ) {
  for ( auto &s : signals )
    if ( s.uuid() == uuid )
      return s.status();
  return SignalStatus::UNDEFINED;
}

GroupStatus SignalGroupImp::status() {
  if ( loopRunning && stopRequest )
    return GroupStatus::STOP_REQUEST;

  if ( loopRunning )
    return GroupStatus::RUNNING;

  return GroupStatus::IDLE;
}

std::size_t SignalGroupImp::size() {
  return signals.size();
}

void SignalGroupImp::eventLoop() {
  Connection c;
  sd_bus *bus = (sd_bus *)c.borrowBusObject();
  struct pollfd p;
  int r = 0;
  uint64_t usec;

  p.fd = sd_bus_get_fd( bus );
  sd_bus_get_timeout( bus, &usec );
  p.fd = sd_bus_get_fd( bus );

  while ( !stopRequest ) {
    loopRunning = true;
    if ( signalChanged ) {
      std::lock_guard<std::mutex> lock( mutex );
      signalChanged = false;

      for ( size_t i = 0; i < signals.size(); ++i ) {
        if ( signals[i].status() == SignalStatus::REMOVE_REQUEST ) {
          void *slot = signals[i].slot();
          slot = ::sd_bus_slot_unref( (sd_bus_slot *)slot );
          signals[i].updateSlot( slot );
          signals[i].updateStatus( SignalStatus::REMOVED );
          signals.erase( signals.begin() + i );
        }

        if ( signals[i].status() == SignalStatus::ADD_REQUEST ) {
          std::string rule = signals[i].rule();
          void *userdata = &signals[i];
          void *slot = signals[i].slot();
          r = ::sd_bus_add_match(
            bus, (sd_bus_slot **)&slot, rule.c_str(), match_callback, userdata );
          if ( r < 0 ) {
            signals[i].updateStatus( SignalStatus::MATCH_FAILED );
          } else {
            signals[i].updateStatus( SignalStatus::ADDED );
          }
        }
      }
    }

    r = ::sd_bus_process( bus, NULL );
    if ( r > 0 )
      continue;  // something's available, no need to poll events

    p.events = static_cast<short int>( ::sd_bus_get_events( bus ) );
    poll( &p, 1, 5000 );  // time in milliseconds
  }  // main while

  // clean up before exit the event_loop:
  // change the status from ADDED to REQUEST, so the next event_loop
  // starts a new match for each signal.
  for ( auto &s : signals ) {
    void *slot = s.slot();
    if ( slot ) {
      slot = ::sd_bus_slot_unref( (sd_bus_slot *)slot );
      s.updateSlot( slot );
    }
    if ( s.status() == SignalStatus::ADDED )
      s.updateStatus( SignalStatus::ADD_REQUEST );
  }

  /* valgrin reports memory leak because it thinks
   * the bus pointer never got deallocated. This is not true
   * since Connection deallocates the bus when destructed.
   * To prove the point, adding the following line of code
   * removes valgrind warning!
   * bus = ::sd_bus_flush_close_unref(bus);
   * Note that Connection cleans the bus connection in the destructor
   */
  loopRunning = false;
  stopRequest = false;
}

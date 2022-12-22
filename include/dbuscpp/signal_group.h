#pragma once
#include "dbuscpp/common.h"
#include "dbuscpp/connection.h"
#include "dbuscpp/signal.h"
#include <boost/uuid/uuid.hpp>
#include <mutex>
#include <thread>
#include <vector>

namespace dbus {

using SignalCallback = std::function<void( SignalID )>;

enum GroupStatus {
  IDLE = 0,
  RUNNING = 1,
  STOP_REQUEST = 2,
};

class SignalGroupImp {
public:
  static SignalGroupImp& get() {
    static SignalGroupImp sg;
    return sg;
  }
  SignalGroupImp( const SignalGroupImp& ) = delete;
  SignalGroupImp& operator=( const SignalGroupImp& ) = delete;
  ~SignalGroupImp();
  void start();
  SignalID createSignal();
  bool matchRule( SignalID uuid, std::string rule );
  bool signalCallback( SignalID uuid, std::function<void( SignalID )> callback );
  bool signalStatusCallback( SignalID uuid, std::function<void( SignalID )> callback );
  bool add( SignalID uuid );
  bool contains( SignalID uuid );
  void remove( SignalID uuid );
  SignalStatus status( SignalID uuid );
  GroupStatus status();
  std::size_t size();

private:
  SignalGroupImp();
  void eventLoop();
  void stop();

  std::vector<Signal> signals;

  std::mutex mutex;
  std::thread loopThread;

  bool signalChanged = false;
  bool loopRunning = false;
  bool stopRequest = false;
};

namespace SignalGroup {

inline void start() {
  SignalGroupImp::get().start();
}

inline SignalID createSignal() {
  return SignalGroupImp::get().createSignal();
}

inline bool matchRule( SignalID uuid, std::string rule ) {
  return SignalGroupImp::get().matchRule( uuid, rule );
}

inline bool signalCallback( SignalID uuid, std::function<void( SignalID )> callback ) {
  return SignalGroupImp::get().signalCallback( uuid, callback );
}

inline void signalStatusCallback( SignalID uuid, std::function<void( SignalID )> callback ) {
  SignalGroupImp::get().signalStatusCallback( uuid, callback );
}

inline void add( SignalID uuid ) {
  SignalGroupImp::get().add( uuid );
}

inline bool contains( SignalID uuid ) {
  return SignalGroupImp::get().contains( uuid );
}

inline void remove( SignalID uuid ) {
  SignalGroupImp::get().remove( uuid );
}

inline int status( SignalID uuid ) {
  return SignalGroupImp::get().status( uuid );
}

inline int status() {
  return SignalGroupImp::get().status();
}

inline std::size_t size() {
  return SignalGroupImp::get().size();
}

}  // namespace SignalGroup
}  // namespace dbus

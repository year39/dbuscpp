#include <dbuscpp/dbuscpp.h>
#include <iostream>
#include <string>

using namespace dbus;

inline void wait_for( int seconds ) {
  std::this_thread::sleep_for( std::chrono::seconds( seconds ) );
}

int main() {
  Manager manager;
  Reply r1;
  r1 = manager.propertyGet( "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "Name" );

  r1.enterContainer();
  std::string s;
  r1.read( s );
  std::cout << s << "\n";

  Message m2 = manager.methodCall( "org.freedesktop.UPower",
    "/org/freedesktop/UPower",
    "org.freedesktop.UPower",
    "GetCriticalAction" );
  Reply r2 = manager.call( m2 );

  std::cout << "reply type: " << r2.signatureType() << ", " << r2.signatureContents() << ": ";
  std::string a;
  r2.read( a );
  std::cout << a << "\n";

  manager.propertySetDirect(
    "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "Discoverable", false );

  std::string ss = "ubuntu2";
  try {
    manager.propertySetDirect( "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "Alias", ss );
  } catch ( std::runtime_error &e ) {
    std::cout << e.what() << "\n";
  }

  // list of objects
  std::vector<ObjectPath> objects;
  try {
    objects = manager.objects( "org.bluez" );
  } catch ( std::runtime_error &e ) {
    std::cout << e.what();
  }

  for ( auto &i : objects ) {
    std::cout << i << ":\n";
  }

  try {
    std::vector<std::string> filters;
    Message m3 = manager.methodCall(
      "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "GetDiscoveryFilters" );
    Reply r3 = manager.call( m3 );
    r3.read( filters );
    for ( auto &i : filters ) {
      std::cout << i << "\n";
    }
  } catch ( std::runtime_error &e ) {
    std::cout << e.what() << "\n";
  }

  return 0;
}

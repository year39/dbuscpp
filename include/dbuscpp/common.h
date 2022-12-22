#pragma once
#include <string>
#include <vector>

namespace dbus {

class ObjectPath : public std::string {
public:
  ObjectPath() : std::string {} {}
  ObjectPath( std::string objectPath ) : std::string { objectPath } {}
};

enum DATA_TYPE : char {
  EMPTY = ' ',
  BYTE = 'y',
  BOOLEAN = 'b',
  INT16 = 'n',
  UINT16 = 'q',
  INT32 = 'i',
  UINT32 = 'u',
  INT64 = 'x',
  UINT64 = 't',
  DOUBLE = 'd',
  STRING = 's',
  OBJECT_PATH = 'o',
  SIGNATURE = 'g',
  UNIX_FD = 'h',
  ARRAY = 'a',
  VARIANT = 'v',
  STRUCT = 'r',
  STRUCT_BEGIN = '(',
  STRUCT_END = ')',
  DICT_ENTRY = 'e',
  DICT_ENTRY_BEGIN = '{',
  DICT_ENTRY_END = '}'
};

enum MessageType { INVALID = 0, METHOD_CALL, METHOD_RETURN, METHOD_ERROR, SIGNAL };

}  // namespace dbus

#pragma once

#include <string>
#include <vector>

class Key;
typedef std::vector<Key> Keys;

class Key
{
public:

  enum Type
  {
    kMinor,
    kMajor
  };

  //std::string short_name;
  int         num;
  Type        type;
  //std::string name;
  //std::string alternate;

  Key() : num(0), type(static_cast<Type>(0)) {}
  Key(int num, Type type/*, std::string name, std::string short_name, std::string alternate*/) :
    num(num), type(type)/*, name(name), short_name(short_name), alternate(alternate)*/ {}

  bool operator==(Key const& key) const;
  Key  operator +(int semitones) const;
  Key  operator -(int semitones) const;
  bool operator <(Key const& k) const;

  static std::string GetName(
    int num,
    Key::Type type
    );

  static std::string GetShortName(
    int num,
    Key::Type type
    );

  static std::string GetAlternateName(
    int num,
    Key::Type type
    );

  static Keys const& GetKeys();
  static Keys const& GetOrdering(Type type);
  static Key  KeyFromString(std::string const& str);
  static Key  GetShiftedKey(Key const& original_key, double old_bpm, double new_bpm);
  static int  GetKeyIndex(Key const& key);
  static int  GetCamelotDistance(Key const& k1, Key const& k2);
  static int  GetTransposeDistance(Key const& k1, Key const& k2);
  static bool AreCompatibleKeys(Key const& k1, Key const& k2);
  static void GetCompatibleKeys(Key const& key, Keys& compatible);
  static Key  GetKey(int num, Type type);
};
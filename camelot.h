#ifndef CAMELOT_H
#define CAMELOT_H

#include <string>
#include <vector>

class Camelot
{
public:

  enum Type
  {
    kMinor,
    kMajor
  };

  struct Key
  {
    std::string short_name;
    int         num;
    Type        type;
    std::string name;
    std::string alternate;

    Key(int num, Type type, std::string name, std::string short_name, std::string alternate) : num(num), type(type), name(name), short_name(short_name), alternate(alternate) {}
    bool operator==(Camelot::Key const& key) const;
    Key operator+(int semitones) const;
    Key operator-(int semitones) const;
    bool operator<(Camelot::Key const& k) const;
  };

  typedef std::vector<Key> Keys;

  static Keys const& GetKeys();
  static Keys const& GetOrdering(Type type);
  static Key  KeyFromString(std::string const& str);
  static Key  GetShiftedKey(Key const& original_key, double old_bpm, double new_bpm);
  static int  GetKeyIndex(Key const& key);
  static int  GetCamelotDistance(Key const& k1, Key const& k2);
  static int  GetTransposeDistance(Key const& k1, Key const& k2);
  static bool AreCompatibleKeys(Key const& k1, Key const& k2);
  static Keys GetCompatibleKeys(Key const& key);
  static Key  GetKey(int num, Type type);
};

#endif
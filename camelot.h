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
  };

  typedef std::vector<Key> Keys;

  static Keys GetKeys();
  static Key  KeyFromString(std::string const& str);
  static Key  GetShiftedKey(Camelot::Key const& original_key, double old_bpm, double new_bpm);
  static int  GetKeyIndex(Camelot::Key const& key);
  static int  GetCamelotDistance(Camelot::Key const& k1, Camelot::Key const& k2);

private:
  static Keys keys;
};

#endif
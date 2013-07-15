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
    int         num;
    Type        type;
    std::string name;
    std::string short_name;
    std::string alternate;

    Key(int num, Type type, std::string name, std::string short_name, std::string alternate) : num(num), type(type), name(name), short_name(short_name), alternate(alternate) {}
  };

  typedef std::vector<Key> Keys;

  static Keys GetKeys();
  static Key KeyFromString(std::string const& str);

private:
  static Keys keys;
};

#endif
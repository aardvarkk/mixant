#include <exception>

#include "camelot.h"

Camelot::Keys Camelot::keys;

Camelot::Keys Camelot::GetKeys()
{
  if (!keys.empty()) {
    return keys;
  }

  keys.push_back(Key(1, kMinor, "A-Flat Minor", "Abm", "G#m"));
  keys.push_back(Key(1, kMajor, "B Major", "B", ""));
  keys.push_back(Key(2, kMinor, "E-Flat Minor", "Ebm", "D#m"));
  keys.push_back(Key(2, kMajor, "F-Sharp Major", "F#", "Gb"));
  keys.push_back(Key(3, kMinor, "B-Flat Minor", "Bbm", "A#m"));
  keys.push_back(Key(3, kMajor, "D-Flat Major", "Db", "C#"));
  keys.push_back(Key(4, kMinor, "F Minor", "Fm", ""));
  keys.push_back(Key(4, kMajor, "A-Flat Major", "Ab", "G#"));
  keys.push_back(Key(5, kMinor, "C Minor", "Cm", ""));
  keys.push_back(Key(5, kMajor, "E-Flat Major", "Eb", "D#"));
  keys.push_back(Key(6, kMinor, "G Minor", "Gm", ""));
  keys.push_back(Key(6, kMajor, "B-Flat Major", "Bb", "A#"));
  keys.push_back(Key(7, kMinor, "D Minor", "Dm", ""));
  keys.push_back(Key(7, kMajor, "F Major", "F", ""));
  keys.push_back(Key(8, kMinor, "A Minor", "Am", ""));
  keys.push_back(Key(8, kMajor, "C Major", "C", ""));
  keys.push_back(Key(9, kMinor, "E Minor", "Em", ""));
  keys.push_back(Key(9, kMajor, "G Major", "G", ""));
  keys.push_back(Key(10, kMinor, "B Minor", "Bm", ""));
  keys.push_back(Key(10, kMajor, "D Major", "D", ""));
  keys.push_back(Key(11, kMinor, "F-Sharp Minor", "F#m", "Gbm"));
  keys.push_back(Key(11, kMajor, "A Major", "A", ""));
  keys.push_back(Key(12, kMinor, "D-Flat Minor", "Dbm", "C#m"));
  keys.push_back(Key(12, kMajor, "E Major", "E", ""));

  return keys;
}

Camelot::Key Camelot::KeyFromString(std::string const& str)
{
  Keys keys = Camelot::GetKeys();
  
  for (auto i : keys) {
    if (!i.name.compare(str)) {
      return i;
    } else if (!i.short_name.compare(str)) {
      return i;
    } else if (!i.alternate.compare(str)) {
      return i;
    }
  }

  throw "Invalid key: " + str;
}

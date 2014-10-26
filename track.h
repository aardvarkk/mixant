#ifndef TRACK_H
#define TRACK_H

#include <string>

#include "key.h"

struct Track
{
  std::string name;
  double bpm;
  Key key;

  Track(std::string const& name, double bpm, Key const& key) : name(name), bpm(bpm), key(key) {}
  bool operator==(Track const& t) const { return !name.compare(t.name); }
};

typedef std::vector<Track> Tracks;

#endif
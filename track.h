#ifndef TRACK_H
#define TRACK_H

#include <string>

#include "camelot.h"

class Track
{
public:
  Track(std::string const& name, double bpm, Camelot::Key const& key) : name(name), bpm(bpm), key(key) {}

protected:
  std::string name;
  double bpm;
  Camelot::Key key;
};

typedef std::vector<Track> Tracks;

#endif
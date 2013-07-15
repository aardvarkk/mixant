#ifndef TRACK_H
#define TRACK_H

#include <string>

#include "camelot.h"

struct Track
{
  std::string name;
  double bpm;
  Camelot::Key key;

  Track(std::string const& name, double bpm, Camelot::Key const& key) : name(name), bpm(bpm), key(key) {}
};

typedef std::vector<Track> Tracks;

#endif
#ifndef TRACK_H
#define TRACK_H

#include <string>

#include "key.h"

struct Track
{
  //std::string name;
  int idx;
  double bpm;
  Key key;

  //Track(std::string const& name, double bpm, Key const& key) : name(name), bpm(bpm), key(key) {}
  Track(int idx, double bpm, Key const& key) : idx(idx), bpm(bpm), key(key) {}
  bool operator==(Track const& t) const { return idx == t.idx; }
};

typedef std::vector<Track> Tracks;

#endif
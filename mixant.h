#ifndef MIXANT_H
#define MIXANT_H

#include "mix.h"
#include "track.h"

typedef std::vector< std::vector<double> > Matrix;

struct TrackSpot
{
  TrackSpot& operator=(TrackSpot const& ts)
  {
    this->track = ts.track;
    this->idx = ts.idx;
    return *this;
  }

  TrackSpot(Track const* track, int idx) : track(track), idx(idx) {}

  Track const* track;
  int idx;
};

typedef std::vector<TrackSpot> TrackOrder;

class MixAnt
{
public:

  Mix FindMix(Tracks const& tracks, double* min_dist_val = nullptr);

protected:
  
  Mix MakeMix(TrackOrder const& order);

  Matrix FindTrackDistances(Tracks const& tracks);

  Matrix distances;
  Matrix pheromone;
};

#endif
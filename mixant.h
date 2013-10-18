#ifndef MIXANT_H
#define MIXANT_H

#include "mix.h"
#include "track.h"

typedef std::vector< std::vector<double> > Matrix;

static const int kMixRuns = 1000;
static const int kNumAnts = 1000;
static const double kPheromoneDrop = 2.0 / kMixRuns;
static const double kPheromonePop = 2 * kPheromoneDrop;
static const double kDistThreshold = 2;

struct TrackSpot
{
  TrackSpot() : track(nullptr), idx(-1) {}

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

  Mix FindMix(Tracks const& tracks);
  
  static double FindDistance(
    double bpm_a,
    double bpm_b,
    Camelot::Key const& key_a,
    Camelot::Key const& key_b
    );

  static double FindTrackDistance(
    Track const& a,
    Track const& b
    );

protected:
  
  //Mix MakeMix(TrackOrder const& order);

  Matrix FindTrackDistances(Tracks const& tracks);

  Matrix distances;
  Matrix pheromone;
};

#endif
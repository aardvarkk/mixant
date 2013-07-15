#ifndef MIXANT_H
#define MIXANT_H

#include "mix.h"
#include "track.h"

typedef std::vector< std::vector<double> > Matrix;

class MixAnt
{
public:

  Mix FindMix(Tracks const& tracks);

protected:
  
  typedef std::vector< std::pair<Track, int> > TrackOrder;
  Mix MakeMix(TrackOrder const& order);

  Matrix FindTrackDistances(Tracks const& tracks);

  Matrix distances;
  Matrix pheromone;
};

#endif
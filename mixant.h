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

  Matrix FindTrackDistances(Tracks const& tracks);

  Matrix distances;
  Matrix pheromone;
};

#endif
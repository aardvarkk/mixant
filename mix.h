#ifndef MIX_H
#define MIX_H

#include <iostream>
#include <vector>

#include "track.h"

struct MixStep
{
public:

  Track        track;
  double       bpm_beg;
  double       bpm_end;

  MixStep(Track const& track);
  Key GetPlayKey() const;
  void SetPlayKey(Key const& key);
  int GetTuning() const;

protected:

  Key play_key;
  int          tuning;
};

typedef std::vector<MixStep> MixSteps;

struct Mix
{
  double CalculateDistance();

  MixSteps steps;

  friend std::ostream& operator<<(std::ostream& out, const Mix& mix);
};

#endif
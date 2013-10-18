#include "mix.h"
#include "mixant.h"
#include "utils.h"

#include <boost/math/special_functions/round.hpp>
#include <string>

static const double kBreakCost = 10 * kDistThreshold;

MixStep::MixStep(Track const& track) : track(track), bpm_beg(track.bpm), bpm_end(track.bpm), play_key(track.key), tuning(0)
{
}

Camelot::Key MixStep::GetPlayKey() const
{
  return play_key;
}

int MixStep::GetTuning() const
{
  return tuning;
}

void MixStep::SetPlayKey(Camelot::Key const& key)
{
  play_key = key;
  tuning = Camelot::GetTransposeDistance(track.key, play_key);
}

std::ostream& operator<<(std::ostream& out, const Mix& mix)
{
  for (auto i = 0; i < mix.steps.size(); ++i) {
    
    MixStep const& s = mix.steps[i];

    out 
        << s.track.name;
        
    if (!(s.track == BreakTrack)) {
      out
        << std::endl
        << s.track.key.short_name << " -> " << s.GetPlayKey().short_name
        << " (" << std::showpos << s.GetTuning() << ") " 
        << std::endl
        << std::noshowpos << s.bpm_beg << "bpm -> " << s.bpm_end << "bpm";
      }

     out << std::endl << std::endl;
  }

  return out;
}

double Mix::CalculateDistance()
{
  if (steps.size() <= 1) {
    return 0;
  }

  // Walk through all the tracks, calculating the sum of distances along the way
  double dist = 0;
  for (size_t i = 1; i < steps.size(); ++i) {
    if (steps[i-1].track == BreakTrack || steps[i].track == BreakTrack) {
      continue;
    }
    dist += MixAnt::FindTrackDistance(steps[i-1].track, steps[i].track);
  }

  int breaks = 0;
  for (size_t i = 0; i < steps.size(); ++i) {
    breaks += steps[i].track == BreakTrack;
  }
  dist += kBreakCost * breaks;

  return dist;
}

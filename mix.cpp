#include "mix.h"
#include "mixant.h"
#include "utils.h"

#include <string>

MixStep::MixStep(Track const& track) : track(track), bpm_beg(track.bpm), bpm_end(track.bpm), play_key(track.key), tuning(0)
{
}

Key MixStep::GetPlayKey() const
{
  return play_key;
}

int MixStep::GetTuning() const
{
  return tuning;
}

void MixStep::SetPlayKey(Key const& key)
{
  play_key = key;
  tuning = Key::GetTransposeDistance(track.key, play_key);
}

std::ostream& operator<<(std::ostream& out, const Mix& mix)
{
  for (size_t i = 0; i < mix.steps.size(); ++i) {
    
    MixStep const& s = mix.steps[i];

    out 
        << s.track.name << std::endl
        << s.track.key.short_name << " -> " << s.GetPlayKey().short_name
        << " (" << std::showpos << s.GetTuning() << ") " 
        << std::endl
        << std::noshowpos << s.bpm_beg << "bpm -> " << s.bpm_end << "bpm"
        << std::endl << std::endl;
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
    dist += MixAnt::FindTrackDistance(steps[i-1].track, steps[i].track);
  }

  return dist;
}

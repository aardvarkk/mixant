#include "mix.h"

#include <string>

MixStep::MixStep(Track const& track) : track(track), bpm_beg(track.bpm), bpm_end(track.bpm), play_key(track.key), tuning(0)
{
}

Camelot::Key MixStep::GetPlayKey()
{
  return play_key;
}

int MixStep::GetTuning()
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
  for (auto s : mix.steps) {
    out 
        << s.track.name
        << std::endl
        << s.track.key.short_name << " -> " << s.GetPlayKey().short_name
        << " (" << s.GetTuning() << ") " 
        << std::endl
        << s.bpm_beg << "bpm -> " << s.bpm_end << "bpm"
        << std::endl << std::endl;
  }
  return out;
}

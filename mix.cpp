#include "mix.h"
#include "mixant.h"

#include <string>

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

    if (i > 0) {
      double total_dist, bpm_dist, key_dist;
      MixAnt::FindTrackDistance(mix.steps[i-1].track, mix.steps[i].track, total_dist, &bpm_dist, &key_dist);
      out 
        << "BPM Distance: " << bpm_dist << std::endl
        << "Key Distance: " << key_dist << std::endl
        << "Tot Distance: " << total_dist << std::endl << std::endl;
    }

    out 
        << s.track.name
        << std::endl
        << s.track.key.short_name << " -> " << s.GetPlayKey().short_name
        << " (" << std::showpos << s.GetTuning() << ") " 
        << std::endl
        << std::noshowpos << s.bpm_beg << "bpm -> " << s.bpm_end << "bpm" << " (" << s.track.bpm << " bpm)"
        << std::endl << std::endl;
  }
  return out;
}

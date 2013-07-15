#include "mixant.h"
#include "utils.h"

static double kBPM_diff_multiplier = 1.0;
static double kCamelot_diff_multiplier = 0.5;
  
Matrix MixAnt::FindTrackDistances(Tracks const& tracks)
{
  Matrix dists;
  dists.clear();
  
  dists.resize(tracks.size());
  for (size_t i = 0; i < tracks.size(); ++i) {
    dists[i].resize(tracks.size());
  }

  for (size_t i = 0; i < tracks.size(); ++i) {
    for (size_t j = i+1; j < tracks.size(); ++j) {
      // Get the BPM portion (in semitones)
      double max_bpm      = std::max(tracks[i].bpm, tracks[j].bpm);
      double min_bpm      = std::min(tracks[i].bpm, tracks[j].bpm);
      double bpm_diff     = max_bpm - min_bpm;
      double bpm_ratio    = (min_bpm + bpm_diff) / min_bpm;
      double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

      // Get the key portion _after_ BPM match (in semitones)
      double target_bpm = min_bpm + bpm_diff / 2;
      Camelot::Key new_key_i = Camelot::GetShiftedKey(tracks[i].key, tracks[i].bpm, target_bpm);
      Camelot::Key new_key_j = Camelot::GetShiftedKey(tracks[j].key, tracks[j].bpm, target_bpm);
      int camelot_dist = Camelot::GetCamelotDistance(new_key_i, new_key_j);

      double dist = kBPM_diff_multiplier * bpm_ratio_st + kCamelot_diff_multiplier * camelot_dist;
      dists[i][j] = dist;
      dists[j][i] = dist;
    }
  }
  return dists;
}

Mix MixAnt::FindMix(Tracks const& tracks)
{
  distances = FindTrackDistances(tracks);

  Mix m;
  return m;
}

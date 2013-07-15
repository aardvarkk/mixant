#include <random>

#include "mixant.h"
#include "utils.h"

static std::tr1::mt19937 eng;

static int kNumAnts = 10000;
static double kBPM_diff_multiplier = 1.0;
static double kCamelot_diff_multiplier = 0.5;
  
Matrix MixAnt::FindTrackDistances(Tracks const& tracks)
{
  Matrix dists;
  
  dists.resize(tracks.size());
  for (auto i = dists.begin(); i != dists.end(); ++i) {
    i->resize(tracks.size());
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
      double target_bpm = min_bpm / 2;
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

  // Just for curiosity, find the minimum distance
  double min_diff = DBL_MAX;
  size_t min_i, min_j;
  for (size_t i = 0; i < distances.size(); ++i) {
    for (size_t j = i + 1; j < distances.size(); ++j) {
      if (distances[i][j] < min_diff) {
        min_diff = distances[i][j];
        min_i = i; min_j = j;
      }
    }
  }

  // Make a pheromone matrix -- it need not be symmetrical, because index i,j is FROM i TO j
  pheromone.resize(tracks.size());
  for (size_t i = 0; i < pheromone.size(); ++i) {
    pheromone[i].resize(tracks.size(), 1);
    pheromone[i][i] = 0;
  }
  
  // For some set number of iterations...
  Mix best_mix;
  for (;;) {

    // Run a set number of ants through the entire collection, creating a mix
    for (int i = 0; i < kNumAnts; ++i) {

      // Which tracks are available...
      std::vector<bool> available(tracks.size(), true);

      // Ant chooses a random starting point...
      std::tr1::uniform_int<int> rnd_cur(0, tracks.size() - 1);
      int cur_track = rnd_cur(eng);
      available[cur_track] = false;

      // Make an array of all available indices
      std::vector<int> available_idx;
      for (size_t j = 0; j < available.size(); ++j) {
        if (available[j]) {
          available_idx.push_back(j);
        }
      }

      // Add up the values of all the pheromones
      std::vector<double> running_sum(available_idx.size(), pheromone[cur_track][available_idx.front()]);
      for (size_t j = 1; j < available_idx.size(); ++j) {
        running_sum[j] = pheromone[cur_track][available_idx[j]] + running_sum[j-1];
      }

      // Choose an available next track based on pheromone amounts
      std::tr1::uniform_real<double> rnd_nxt(0, running_sum.back());
      double rnd_nxt_val = rnd_nxt(eng);
      int chosen_available_idx = 0;
      while (running_sum[chosen_available_idx] < rnd_nxt_val) {
        chosen_available_idx++;
      }
      int nxt_track = available_idx[chosen_available_idx];
    }

    // Score the mix based on the sum of all distances
  }

  return best_mix;
}

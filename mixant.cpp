#include <iostream>
#include <random>

#include "mixant.h"
#include "utils.h"

static std::tr1::mt19937 eng;
static int kMixRuns = 500;
static int kNumAnts = 1000;
static double kPheromoneDrop = 2.0 / kMixRuns;
static double kPheromonePop = 2 * kPheromoneDrop;

// Find distance from one track to another
// We assume that the first track is "already playing", so how much do you have to adjust
// the second track to match with the first?
void MixAnt::FindTrackDistance(Track const& a, Track const& b, double& total_dist, double* bpm_dist, double* key_dist)
{
  // Get the (directional!) BPM portion (in semitones)
  // It can be positive or negative depending on whether we're speeding up or slowing down
  // That way, the distances are more careful that you're not slowing a song down (reducing pitch)
  // but also tuning it up (increasing pitch).
  double bpm_ratio    = a.bpm / b.bpm;
  double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

  // How far must we transpose the "second" track to make it compatible with the "first"?
  int min_transpose_dist = INT_MAX;
  for (auto k : Camelot::GetCompatibleKeys(a.key)) {
    // Only care about keys compatible with "first", and ones of the same type as our "second" track
    if (k.type != b.key.type) {
      continue;
    }
    int transpose_dist = Camelot::GetTransposeDistance(b.key, k);
    if (abs(transpose_dist) < abs(min_transpose_dist)) {
      min_transpose_dist = transpose_dist;
    }
  }

  double this_bpm_dist = bpm_ratio_st;
  double this_key_dist = min_transpose_dist;
  total_dist = abs(this_bpm_dist - this_key_dist) + std::max(abs(this_bpm_dist), abs(this_key_dist));

  if (bpm_dist) {
    *bpm_dist = this_bpm_dist;
  }
  if (key_dist) {
    *key_dist = this_key_dist;
  }
}

Matrix MixAnt::FindTrackDistances(Tracks const& tracks)
{
  Matrix dists;

  dists.resize(tracks.size());
  for (auto i = dists.begin(); i != dists.end(); ++i) {
    i->resize(tracks.size());
  }

  for (size_t i = 0; i < tracks.size(); ++i) {
    for (size_t j = i+1; j < tracks.size(); ++j) {
      
      double total_dist;
      MixAnt::FindTrackDistance(tracks[i], tracks[j], total_dist); 

      dists[i][j] = total_dist;
      dists[j][i] = total_dist;
    }
  }
  return dists;
}

Mix MixAnt::FindMix(Tracks const& tracks, double* min_dist_val)
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
  }

  // Best distance and order found...
  double min_dist = DBL_MAX;
  TrackOrder min_order;

  // For some set number of iterations...
  for (int r = 0; r < kMixRuns; ++r) {

    // Best distance found this run
    double min_run_dist = DBL_MAX;
    TrackOrder min_run_order;

    // Run a set number of ants through the entire collection, creating a mix
    for (int i = 0; i < kNumAnts; ++i) {

      // The track order we chose
      TrackOrder order;
      order.reserve(tracks.size());

      // Which tracks are available...
      std::vector<bool> available(tracks.size(), true);

      // Ant chooses a starting point based on pheromones...
      std::vector<double> running_sum(tracks.size(), pheromone.front().front());
      for (size_t j = 1; j < tracks.size(); ++j) {
        running_sum[j] = pheromone[j][j] + running_sum[j-1];
      }
      std::tr1::uniform_real<double> rnd_cur(0, running_sum.back());
      double rnd_cur_val = rnd_cur(eng);
      int cur_track = 0;
      while (running_sum[cur_track] < rnd_cur_val) {
        cur_track++;
      }
      available[cur_track] = false;
      order.emplace_back(TrackSpot(&tracks[cur_track], cur_track));

      // Total "song distance" walked by this ant
      double total_dist = 0;
      for (size_t k = 1; k < tracks.size(); ++k) {

        // Make an array of all available indices
        std::vector<int> available_idx;
        available_idx.reserve(available.size());
        for (size_t j = 0; j < available.size(); ++j) {
          if (available[j]) {
            available_idx.push_back(j);
          }
        }

        // Choose an available next track based on pheromone amounts
        std::vector<double> running_sum(available_idx.size(), pheromone[cur_track][available_idx.front()]);
        for (size_t j = 1; j < available_idx.size(); ++j) {
          running_sum[j] = pheromone[cur_track][available_idx[j]] + running_sum[j-1];
        }
        std::tr1::uniform_real<double> rnd_nxt(0, running_sum.back());
        double rnd_nxt_val = rnd_nxt(eng);
        int chosen_available_idx = 0;
        while (running_sum[chosen_available_idx] < rnd_nxt_val) {
          chosen_available_idx++;
        }
        int nxt_track = available_idx[chosen_available_idx];
        available[nxt_track] = false;

        // Adjust our distance 
        // OPTION 1: take into account the distance along the whole path
        total_dist += distances[cur_track][nxt_track];

        // OPTION 2: take into account the worst case distance
        //total_dist = std::max(total_dist, distances[cur_track][nxt_track]);

        // For next time through the loop
        cur_track = nxt_track;
        order.emplace_back(TrackSpot(&tracks[cur_track], cur_track));
      }

      // OPTION 1: Normalize for the number of tracks
      total_dist /= tracks.size();

      // This is the best this run
      if (total_dist < min_run_dist) {
        min_run_dist = total_dist;
        min_run_order = order;
        //std::cout << "New min run dist: " << min_run_dist << std::endl;
      }

      // We did better than we EVER have
      if (total_dist < min_dist) {
        min_dist = total_dist;
        min_order = order;
        std::cout << "New min dist: " << min_dist << std::endl;
      }
    }

    // Lay down a set amount of pheromone along the track from this ant, and reduce all pheromone by twice that amount

    // Reward the start track
    pheromone[min_run_order.front().idx][min_run_order.front().idx] += kPheromonePop;

    // Reward all choices along the route (edges from i->j)
    for (size_t i = 1; i < min_run_order.size(); ++i) {
      pheromone[min_run_order[i-1].idx][min_run_order[i].idx] += kPheromonePop;
    }

    // Now drop everything across the board
    for (size_t i = 0; i < pheromone.size(); ++i) {
      for (size_t j = 0; j < pheromone[i].size(); ++j) {
        pheromone[i][j] = std::max(0.0, pheromone[i][j] - kPheromoneDrop);
      }
    }

    std::cout << "Finished run " << r+1 << " of " << kMixRuns << std::endl;
  }

  std::cout << "Final min dist: " << min_dist << std::endl;

  if (min_dist_val) {
    *min_dist_val = min_dist;
  }

  return MakeMix(min_order);
}

Mix MixAnt::MakeMix(TrackOrder const& order)
{
  Mix mix;

  // If there's only one track, we're done!
  if (order.size() < 2) {
    mix.steps.push_back(*order.front().track);
    return mix;
  }

  // For now, just play the first song and only change its BPM
  // such that it ends at a balance between its BPM and the next BPM
  MixStep prv_ms(*order.front().track);
  prv_ms.bpm_end = (prv_ms.bpm_beg + order[1].track->bpm) / 2;
  mix.steps.push_back(prv_ms);

  // Create instructions for the mix
  // Look at the previous and next at each step
  for (size_t i = 1; i < order.size()-1; ++i) {

    // Our current and next tracks
    Track const* cur = order[i].track;
    Track const* nxt = i < order.size() - 1 ? order[i+1].track : nullptr;

    // Set up our current mix step
    MixStep cur_ms(*cur);
    cur_ms.bpm_beg = prv_ms.bpm_end;

    // If we have a next track...
    if (nxt) {
      cur_ms.bpm_end = (cur->bpm + nxt->bpm) / 2;
    }

    // Is the current track compatible with the previous?
    bool compatible = Camelot::AreCompatibleKeys(cur_ms.track.key, prv_ms.GetPlayKey());

    // We're done if we're already compatible
    if (compatible) {
      mix.steps.push_back(cur_ms);
      prv_ms = cur_ms;
      continue;
    }

    // We're not compatible, so we need a tuning change in the current track
    // Choose the key with the smallest combined distance between previous and next
    int min_dist = INT_MAX;
    for (auto k : Camelot::GetKeys()) {
      
      // Can't switch between min-maj!
      // And we only care about compatible keys
      if (k.type != cur_ms.track.key.type || !Camelot::AreCompatibleKeys(k, prv_ms.GetPlayKey())) {
        continue;
      }

      // Want the smallest distance between our natural key and the next one
      int cur_dist = Camelot::GetTransposeDistance(cur_ms.track.key, k);
      // Check the total distance -- want the best value
      if (abs(cur_dist) <= min_dist) {
        min_dist = abs(cur_dist);
        cur_ms.SetPlayKey(k);
      }
    }

    mix.steps.push_back(cur_ms);
    prv_ms = cur_ms;
  }

  return mix;
}

#include <iostream>
#include <random>

#include "mixant.h"
#include "utils.h"

static std::tr1::mt19937 eng;
static int kMixRuns = 20;
static int kNumAnts = 50;
static double kPheromoneDrop = 0.01;
static double kPheromonePop = 2 * kPheromoneDrop;
static double kBPMDiffMult = 1;
static double kCamelotDiffMult = 10;

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
      //double target_bpm = min_bpm / 2;
      //Camelot::Key new_key_i = Camelot::GetShiftedKey(tracks[i].key, tracks[i].bpm, target_bpm);
      //Camelot::Key new_key_j = Camelot::GetShiftedKey(tracks[j].key, tracks[j].bpm, target_bpm);
      //int camelot_dist = Camelot::GetCamelotDistance(new_key_i, new_key_j);
      int camelot_dist = Camelot::GetCamelotDistance(tracks[i].key, tracks[j].key);

      double dist = kBPMDiffMult * bpm_ratio_st + kCamelotDiffMult * abs(camelot_dist);
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
  }

  // Best distance and order found...
  double min_dist = DBL_MAX;
  TrackOrder min_order;

  // For some set number of iterations...
  for (int r = 0; r < kMixRuns; ++r) {

    // Run a set number of ants through the entire collection, creating a mix
    for (int i = 0; i < kNumAnts; ++i) {

      // The track order we chose
      TrackOrder order;

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
      order.push_back(std::make_pair(tracks[cur_track], cur_track));

      // Total "song distance" walked by this ant
      double total_dist = 0;

      for (size_t k = 1; k < tracks.size(); ++k) {
        // Make an array of all available indices
        std::vector<int> available_idx;
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
        total_dist += distances[cur_track][nxt_track];

        // For next time through the loop
        cur_track = nxt_track;
        order.push_back(std::make_pair(tracks[cur_track], cur_track));
      }

      // Score the mix based on the sum of all distances
      if (total_dist < min_dist) {
        min_dist = total_dist;
        min_order = order;
      }
    }

    // Lay down a set amount of pheromone along the track from this ant, and reduce all pheromone by twice that amount

    // Reward the start track
    pheromone[min_order.front().second][min_order.front().second] += kPheromonePop;

    // Reward all choices along the route (edges from i->j)
    for (size_t i = 1; i < min_order.size(); ++i) {
      pheromone[min_order[i-1].second][min_order[i].second] += kPheromonePop;
    }

    // Now drop everything across the board
    for (size_t i = 0; i < pheromone.size(); ++i) {
      for (size_t j = 0; j < pheromone[i].size(); ++j) {
        pheromone[i][j] = std::max(0.0, pheromone[i][j] - kPheromoneDrop);
      }
    }

    std::cout << "Finished run " << r+1 << " of " << kMixRuns << std::endl;
  }

  return MakeMix(min_order);
}

Mix MixAnt::MakeMix(TrackOrder const& order)
{
  Mix mix;

  // If there's only one track, we're done!
  if (order.size() < 2) {
    mix.steps.push_back(order.front().first);
    return mix;
  }

  // For now, just play the first song and only change its BPM
  // such that it ends at a balance between its BPM and the next BPM
  MixStep prv_ms(order.front().first);
  prv_ms.bpm_end = (prv_ms.bpm_beg + order[1].first.bpm) / 2;
  mix.steps.push_back(prv_ms);

  // Create instructions for the mix
  // Look at the previous and next at each step
  for (size_t i = 1; i < order.size()-1; ++i) {

    // Our current and next tracks
    Track const* cur = &order[i].first;
    Track const* nxt = i < order.size() - 1 ? &order[i+1].first : nullptr;

    // Set up our current mix step
    MixStep cur_ms(*cur);
    cur_ms.bpm_beg = prv_ms.bpm_end;

    // If we have a next track...
    if (nxt) {
      cur_ms.bpm_end = (cur->bpm + nxt->bpm) / 2;
    }

    // Is the current track compatible with the previous?
    bool compatible = false;
    for (auto k : Camelot::GetCompatibleKeys(prv_ms.GetPlayKey())) {
      compatible |= Camelot::AreCompatibleKeys(cur_ms.track.key, k);
    }

    // We're done if we're already compatible
    if (compatible) {
      mix.steps.push_back(cur_ms);
      continue;
    }

    // We're not compatible, so we need a tuning change in the current track
    // Choose the key with the smallest combined distance between previous and next
    int min_dist = INT_MAX;
    int min_prv_dist = INT_MAX;
    int min_nxt_dist = INT_MAX;
    for (auto k : Camelot::GetKeys()) {
      // Can't switch between min-maj!
      if (k.type != cur_ms.track.key.type) {
        continue;
      }

      int prv_dist = Camelot::GetCamelotDistance(prv_ms.GetPlayKey(), k);
      int nxt_dist = nxt ? Camelot::GetCamelotDistance(k, nxt->key) : 0;
      // Check the total distance -- want the best value
      if (abs(prv_dist) + abs(nxt_dist) <= min_dist) {
        // Prefer to stay closer to what we've already played
        if (abs(prv_dist) < abs(min_prv_dist)) {
          min_dist = abs(prv_dist) + abs(nxt_dist);
          min_prv_dist = prv_dist;
          min_nxt_dist = nxt_dist;
          cur_ms.SetPlayKey(k);
        }
      }
    }

    mix.steps.push_back(cur_ms);

    // Set our previous for the next time through
    prv_ms = cur_ms;
  }

  return mix;
}

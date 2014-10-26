#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>

#include "mixant.h"
#include "utils.h"

static std::tr1::mt19937 eng;

double MixAnt::FindDistance(
  double bpm_a,
  double bpm_b,
  Key const& key_a,
  Key const& key_b
  )
{
  // Get the (directional!) BPM portion (in semitones)
  // It can be positive or negative depending on whether we're speeding up or slowing down
  // That way, the distances are more careful that you're not slowing a song down (reducing pitch)
  // but also tuning it up (increasing pitch).
  double bpm_ratio    = bpm_a / bpm_b;
  double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

  // How far must we transpose the "second" track to make it compatible with the "first"?
  int min_transpose_dist = INT_MAX;
  Keys compatible;
  Key::GetCompatibleKeys(key_a, compatible);
  for (auto k : compatible) {
    // Only care about keys compatible with "first", and ones of the same type as our "second" track
    if (k.type != key_b.type) {
      continue;
    }
    int transpose_dist = Key::GetTransposeDistance(key_b, k);
    if (abs(transpose_dist) < abs(min_transpose_dist)) {
      min_transpose_dist = transpose_dist;
    }
  }

  double bpm_dist = bpm_ratio_st;
  double key_dist = min_transpose_dist;
  double tuning_dist = bpm_dist - key_dist;
  return abs(tuning_dist) + std::max(abs(bpm_dist), abs(key_dist));
}

// Find distance from one track to another
// We assume that the first track is "already playing", so how much do you have to adjust
// the second track to match with the first?
double MixAnt::FindTrackDistance(
  Track const& a,
  Track const& b
  )
{
  return FindDistance(a.bpm, b.bpm, a.key, b.key);
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
      
      double total_dist = MixAnt::FindTrackDistance(tracks[i], tracks[j]); 

      dists[i][j] = total_dist;
      dists[j][i] = total_dist;
    }
  }
  return dists;
}

Mix MixAnt::FindMix(Tracks const& tracks)
{
  //eng.seed(static_cast<unsigned long>(time(NULL)));

  Mix best_mix;
  double best_dist = DBL_MAX;
  size_t best_chain = 0;

  // Do a whole bunch of runs
  for (size_t r = 0; r < kMixRuns; ++r) {

    std::cout << "Run " << r+1 << " of " << kMixRuns << std::endl;

    // Try each starting track
    for (size_t i = 0; i < tracks.size(); ++i) {

      // The mix starting with this track
      Mix m;
      MixStep cur_ms(tracks[i]);
      MixStep prv_ms(tracks[i]);
      size_t chain = 1;

      //std::cout << "Trying " << tracks[i].name << " as start..." << std::endl;

      TrackOrder available;
      available.reserve(tracks.size()-1);
      for (size_t j = 0; j < tracks.size(); ++j) {
        if (j == i) {
          continue;
        }
        available.push_back(TrackSpot(&tracks[j], j));
      }

      // Choose the next track
      while (!available.empty()) {

        // Collect all tracks that are within a threshold distance
        // Need to calculate distance to all available tracks
        TrackOrder usable;
        std::vector<double> dists;
        dists.resize(available.size());

        for (size_t k = 0; k < dists.size(); ++k) {
          dists[k] = MixAnt::FindDistance(prv_ms.bpm_beg, available[k].track->bpm, prv_ms.GetPlayKey(), available[k].track->key);
          if (dists[k] < kDistThreshold) {
            usable.push_back(available[k]);
          }
        }

        // We've run out of usable tracks, so we need a hard break
        // We'll choose a random track to continue with
        if (usable.empty()) {
          //std::cout << "End of chain reached -- no more usable tracks" << std::endl;
          break;
        }

        // We've added another track
        ++chain;

        // We have our starting track, so now randomly pick a second one that falls under a distance threshold
        std::tr1::uniform_int<> rnd_usable(0, usable.size() - 1);
        int use_idx = rnd_usable(eng);

        // Create our current mix step, and adjust it to match the previous
        cur_ms = MixStep(*usable[use_idx].track);

        // Adjust previous track to end at this BPM
        prv_ms.bpm_end = cur_ms.bpm_beg;

        // Is the current track compatible with the previous?
        bool compatible = Key::AreCompatibleKeys(cur_ms.track.key, prv_ms.GetPlayKey());

        // We're done if we're already compatible
        if (compatible) {
          goto next;
        }

        // We're not compatible, so we need a tuning change in the current track
        // Choose the key with the smallest combined distance between previous and next
        {
          int min_dist = INT_MAX;
          Keys compatible_keys;
          Key::GetCompatibleKeys(prv_ms.GetPlayKey(), compatible_keys);
          for (auto k : compatible_keys) {

            // Can't switch between min-maj!
            // And we only care about compatible keys
            if (k.type != cur_ms.track.key.type) {
              continue;
            }

            // Want the smallest distance between our natural key and the next one
            int cur_dist = Key::GetTransposeDistance(cur_ms.track.key, k);
            // Check the total distance -- want the best value
            if (abs(cur_dist) <= min_dist) {
              min_dist = abs(cur_dist);
              cur_ms.SetPlayKey(k);
            }
          }
        }

next:
        for (auto it = available.begin(); it != available.end(); ++it) {
          if (it->idx == usable[use_idx].idx) {
            available.erase(it);
            break;
          }
        }
        m.steps.push_back(prv_ms);
        prv_ms = cur_ms;
      }

      m.steps.push_back(cur_ms);

      // How'd we do? Calculate the entire mix distance
      if (chain >= best_chain) {
        double dist = m.CalculateDistance();

        if (chain > best_chain || dist < best_dist) {
          best_chain = chain;
          best_dist = dist;
          best_mix = m;
          std::cout << "Found new best mix of length " << best_chain << " with total distance " << best_dist << std::endl;
        }
      }
    }

  }

  return best_mix;
}

//Mix MixAnt::FindMix(Tracks const& tracks)
//{
//  eng.seed(time(NULL));
//
//  distances = FindTrackDistances(tracks);
//
//  // Just for curiosity, find the minimum distance
//  double min_diff = DBL_MAX;
//  size_t min_i, min_j;
//  for (size_t i = 0; i < distances.size(); ++i) {
//    for (size_t j = i + 1; j < distances.size(); ++j) {
//      if (distances[i][j] < min_diff) {
//        min_diff = distances[i][j];
//        min_i = i; min_j = j;
//      }
//    }
//  }
//
//  // Make a pheromone matrix -- it need not be symmetrical, because index i,j is FROM i TO j
//  pheromone.resize(tracks.size());
//  for (size_t i = 0; i < pheromone.size(); ++i) {
//    pheromone[i].resize(tracks.size(), 1);
//  }
//
//  // Best distance and order found...
//  double min_dist = DBL_MAX;
//  TrackOrder min_order;
//
//  // For some set number of iterations...
//  for (int r = 0; r < kMixRuns; ++r) {
//
//    // Best distance found this run
//    double min_run_dist = DBL_MAX;
//    TrackOrder min_run_order;
//
//    // Run a set number of ants through the entire collection, creating a mix
//    for (int i = 0; i < kNumAnts; ++i) {
//
//      // The track order we chose
//      TrackOrder order;
//      order.reserve(tracks.size());
//
//      // Which tracks are available...
//      std::vector<bool> available(tracks.size(), true);
//
//      // Ant chooses a starting point based on pheromones...
//      std::vector<double> running_sum(tracks.size(), pheromone.front().front());
//      for (size_t j = 1; j < tracks.size(); ++j) {
//        running_sum[j] = pheromone[j][j] + running_sum[j-1];
//      }
//      std::tr1::uniform_real<double> rnd_cur(0, running_sum.back());
//      double rnd_cur_val = rnd_cur(eng);
//      int cur_track = 0;
//      while (running_sum[cur_track] < rnd_cur_val) {
//        cur_track++;
//      }
//      available[cur_track] = false;
//      order.emplace_back(TrackSpot(&tracks[cur_track], cur_track));
//
//      // Total "song distance" walked by this ant
//      double total_dist = 0;
//      for (size_t k = 1; k < tracks.size(); ++k) {
//
//        // Make an array of all available indices
//        std::vector<int> available_idx;
//        available_idx.reserve(available.size());
//        for (size_t j = 0; j < available.size(); ++j) {
//          if (available[j]) {
//            available_idx.push_back(j);
//          }
//        }
//
//        // Choose an available next track based on pheromone amounts
//        std::vector<double> running_sum(available_idx.size(), pheromone[cur_track][available_idx.front()]);
//        for (size_t j = 1; j < available_idx.size(); ++j) {
//          running_sum[j] = pheromone[cur_track][available_idx[j]] + running_sum[j-1];
//        }
//        std::tr1::uniform_real<double> rnd_nxt(0, running_sum.back());
//        double rnd_nxt_val = rnd_nxt(eng);
//        int chosen_available_idx = 0;
//        while (running_sum[chosen_available_idx] < rnd_nxt_val) {
//          chosen_available_idx++;
//        }
//        int nxt_track = available_idx[chosen_available_idx];
//        available[nxt_track] = false;
//
//        // Adjust our distance 
//        // OPTION 1: take into account the distance along the whole path
//        total_dist += distances[cur_track][nxt_track];
//
//        // OPTION 2: take into account the worst case distance
//        //total_dist = std::max(total_dist, distances[cur_track][nxt_track]);
//
//        // For next time through the loop
//        cur_track = nxt_track;
//        order.emplace_back(TrackSpot(&tracks[cur_track], cur_track));
//      }
//
//      // OPTION 1: Normalize for the number of tracks
//      total_dist /= tracks.size();
//
//      // This is the best this run
//      if (total_dist < min_run_dist) {
//        min_run_dist = total_dist;
//        min_run_order = order;
//        //std::cout << "New min run dist: " << min_run_dist << std::endl;
//      }
//
//      // We did better than we EVER have
//      if (total_dist < min_dist) {
//        min_dist = total_dist;
//        min_order = order;
//        std::cout << "New min dist: " << min_dist << std::endl;
//      }
//    }
//
//    // Lay down a set amount of pheromone along the track from this ant, and reduce all pheromone by twice that amount
//
//    // Reward the start track
//    pheromone[min_run_order.front().idx][min_run_order.front().idx] += kPheromonePop;
//
//    // Reward all choices along the route (edges from i->j)
//    for (size_t i = 1; i < min_run_order.size(); ++i) {
//      pheromone[min_run_order[i-1].idx][min_run_order[i].idx] += kPheromonePop;
//    }
//
//    // Now drop everything across the board
//    for (size_t i = 0; i < pheromone.size(); ++i) {
//      for (size_t j = 0; j < pheromone[i].size(); ++j) {
//        pheromone[i][j] = std::max(0.0, pheromone[i][j] - kPheromoneDrop);
//      }
//    }
//
//    std::cout << "Finished run " << r+1 << " of " << kMixRuns << std::endl;
//  }
//
//  std::cout << "Final min dist: " << min_dist << std::endl;
//
//  return MakeMix(min_order);
//}

//Mix MixAnt::MakeMix(TrackOrder const& order)
//{
//  Mix mix;
//
//  mix.max_dist  = 0;
//  mix.min_dist  = DBL_MAX;
//  mix.mean_dist = 0;
//
//  // If there's only one track, we're done!
//  if (order.size() < 2) {
//    mix.steps.push_back(*order.front().track);
//    return mix;
//  }
//
//  // For now, just play the first song and only change its BPM
//  // such that it ends at a balance between its BPM and the next BPM
//  MixStep prv_ms(*order.front().track);
//  prv_ms.bpm_end = (prv_ms.bpm_beg + order[1].track->bpm) / 2;
//  mix.steps.push_back(prv_ms);
//
//  // Create instructions for the mix
//  // Look at the previous and next at each step
//  for (size_t i = 1; i < order.size()-1; ++i) {
//
//    // Our current and next tracks
//    Track const* cur = order[i].track;
//    Track const* nxt = i < order.size() - 1 ? order[i+1].track : nullptr;
//
//    // Set up our current mix step
//    MixStep cur_ms(*cur);
//    cur_ms.bpm_beg = prv_ms.bpm_end;
//
//    // If we have a next track...
//    if (nxt) {
//      cur_ms.bpm_end = (cur->bpm + nxt->bpm) / 2;
//    }
//
//    // Is the current track compatible with the previous?
//    bool compatible = AreCompatibleKeys(cur_ms.track.key, prv_ms.GetPlayKey());
//
//    // We're done if we're already compatible
//    if (compatible) {
//      mix.steps.push_back(cur_ms);
//      prv_ms = cur_ms;
//      continue;
//    }
//
//    // We're not compatible, so we need a tuning change in the current track
//    // Choose the key with the smallest combined distance between previous and next
//    int min_dist = INT_MAX;
//    for (auto k : GetKeys()) {
//      
//      // Can't switch between min-maj!
//      // And we only care about compatible keys
//      if (k.type != cur_ms.track.key.type || !AreCompatibleKeys(k, prv_ms.GetPlayKey())) {
//        continue;
//      }
//
//      // Want the smallest distance between our natural key and the next one
//      int cur_dist = GetTransposeDistance(cur_ms.track.key, k);
//      // Check the total distance -- want the best value
//      if (abs(cur_dist) <= min_dist) {
//        min_dist = abs(cur_dist);
//        cur_ms.SetPlayKey(k);
//      }
//    }
//
//    // Adjust our distances
//    double total_dist;
//    MixAnt::FindTrackDistance(prv_ms.track, cur_ms.track, total_dist);
//
//    mix.min_dist = std::min(mix.min_dist, total_dist);
//    mix.max_dist = std::max(mix.max_dist, total_dist);
//    mix.mean_dist += total_dist;
//
//    mix.steps.push_back(cur_ms);
//    prv_ms = cur_ms;
//  }
//
//  mix.mean_dist /= (mix.steps.size() - 1);
//
//  return mix;
//}

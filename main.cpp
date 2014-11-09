#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "key.h"
#include "mixant.h"
#include "track.h"
#include "utils.h"

static const double kBPMThresh = 0.3;
static const int kKeyShiftThresh = 1;
static const int kMixSongLen = 20;

struct TrackOption
{
  Track track;
  double bpm_ratio;
  int idx;

  TrackOption(Track const& track, double bpm_ratio, int idx) : track(track), bpm_ratio(bpm_ratio), idx(idx) {}
};

//typedef std::map< Key, std::vector<Track> > KeyCount
typedef std::set<Key> KeySet;
typedef std::map<Key, int> KeyCount;

KeySet GetAvailableKeys(KeyCount const& kc)
{
  KeySet available;
  for (auto k : kc) {
    if (k.second <= 0) {
      continue;
    }
    available.insert(k.first);
  }
  return available;
}

bool ChooseKey(Key const& key, KeyCount const& key_count, Keys order, int depth, int& max_depth)
{
  if (depth > max_depth) {
    max_depth = depth;
    std::cout << "New max depth: " << max_depth << std::endl;
  }

  // We've run out of tracks!
  // We've found a potential solution!
  if (GetAvailableKeys(key_count).empty()) {
    return true;
  }

  // Add it to our order, but it's no longer available
  order.push_back(key);

  // Make our own copy to iterate through...
  // Subtract an instance of the key we just used
  KeyCount new_counts(key_count);
  if (--new_counts[key] == 0) {
    new_counts.erase(key);
  }
  
  // Find all compatible tracks, then try to choose each one in turn
  bool found_compatible = false;
  Keys compatible;
  Key::GetCompatibleKeys(key, compatible);
  for (auto k : compatible) {
    if (new_counts.find(k) != new_counts.end()) {
      ChooseKey(k, new_counts, order, depth + 1, max_depth);
      found_compatible = true;
    }
  }

  // Dead end solution...
  return found_compatible;
}

//bool ChooseTrack(Track const& track, Tracks const& available, Tracks order)
//{
//  // We've run out of tracks!
//  // We've found a potential solution!
//  if (available.empty()) {
//    return true;
//  }
//
//  // Add it to our order, but it's no longer available
//  order.push_back(track);
//
//  // Make our own copy to iterate through...
//  Tracks now_available(available);
//  
//  // Erase the one we just took
//  now_available.erase(std::find(now_available.begin(), now_available.end(), track));
//
//  // Find all compatible tracks, then try to choose each one in turn
//  bool found_compatible = false;
//  for (auto t : now_available) {
//    if (Key::AreCompatibleKeys(track.key, t.key)) {
//      ChooseTrack(t, now_available, order);
//      found_compatible = true;
//    }
//  }
//
//  // Dead end solution...
//  return found_compatible;
//}

void GetTracks(std::string const& path, Tracks& tracks, std::vector<std::string>& names)
{
  tracks.clear();

  // Read the file to get our tracks
  std::ifstream ifs(path);
  std::string line;
  int idx = 0;
  while (std::getline(ifs, line)) {
    std::string name = line;

    std::getline(ifs, line);
    Key key = Key::KeyFromString(line);

    double bpm;
    std::getline(ifs, line);
    std::stringstream ss(line);
    ss >> bpm;

    // Allow commenting out tracks
    if (!name.substr(0, 2).compare("//")) {
      continue;
    }

    tracks.push_back(Track(idx++, bpm, key));
    names.push_back(name);
  }
  ifs.close();
}

void GetTracksTabSeparated(std::string const& path, Tracks& tracks, std::vector<std::string>& names)
{
  tracks.clear();

  // Read the file to get our tracks
  std::ifstream ifs(path);
  std::string line;
  int idx = 0;
  while (std::getline(ifs, line)) {
    std::stringstream ss(line);
    
    std::string name;
    std::getline(ss, name, '\t');

    std::string key_str;
    std::getline(ss, key_str, '\t');
    Key key = Key::KeyFromString(key_str);

    double bpm;
    ss >> bpm;

    // Allow commenting out tracks
    if (!name.substr(0, 2).compare("//")) {
      continue;
    }

    tracks.push_back(Track(idx++, bpm, key));
    names.push_back(name);
  }
  ifs.close();
}

void RunTests()
{
  Key Am = Key::KeyFromString("Am");
  Key F = Key::KeyFromString("F");
  assert(Key::GetCamelotDistance(Am, F) == 2);
  assert(Key::GetCamelotDistance(F, Am) == 2);

  Key Fm = Key::KeyFromString("Fm");
  Key Eb = Key::KeyFromString("Eb");
  assert(!Key::AreCompatibleKeys(Fm, Eb));

  Key Db = Key::KeyFromString("Db");
  Key B = Key::KeyFromString("B");
  assert(Key::GetTransposeDistance(Db, B) == -2);

  Key C = Key::KeyFromString("C");
  assert(Key::GetTransposeDistance(B, C) == 1);

  auto keys = Key::GetKeys();
  for (size_t i = 0; i < keys.size(); ++i) {
    for (size_t j = 0; j < keys.size(); ++j) {
      // Can only check keys of the same type (minor/major)
      if (keys[i].type != keys[j].type) {
        continue;
      }
      assert(Key::GetTransposeDistance(keys[i], keys[j]) >= -6 && Key::GetTransposeDistance(keys[i], keys[j]) <= 6);
    }
  }
}

// ASSUME:
// t1 cannot be changed (bpm, key)
// t2 can be changed (only deal with key right now)
// We'll transpose within the key threshold
// We assume tempo we can change kind of however we want within BPM thresholds
// Keys we need to follow so that we don't shift down a bunch and then try to match
// with something high in the next track
bool AreCompatibleTracks(
  Track const& t1, 
  Track const& t2,
  double bpm_thr,
  int key_thr,
  Key& t2_adj,
  double& cost
  )
{
  auto min_bpm = std::min(t1.bpm, t2.bpm);
  auto max_bpm = std::max(t1.bpm, t2.bpm);
  auto bpm_rat = abs(max_bpm / min_bpm);
  bool compatible_bpm = bpm_rat < (1 + bpm_thr);

  if (!compatible_bpm) {
    return false;
  }

  // Find smallest compatible transpose distance from t2's natural key to t1
  int min_shift_dist = INT_MAX;
  bool compatible_keys = false;

  // Try shifting t2's key to all keys within the threshold
  for (int shift = -key_thr; shift <= key_thr; ++shift) {
    Key shifted = t2.key + shift;

    // If they're compatible, mark it!
    if (Key::AreCompatibleKeys(t1.key, shifted)) {
      compatible_keys = true;

      // Better than previous
      if (abs(shift) < min_shift_dist) {
        min_shift_dist = abs(shift);
        t2_adj = shifted;
      }
    }
  }

  // Cost function
  // It's cheaper to adjust tempo (less noticeable) than to adjust key
  cost = (bpm_rat-1) + min_shift_dist;

  return compatible_keys;
}

void ChooseTrack(
  std::vector<std::string> names,
  Tracks& available, 
  Tracks& chosen, 
  int prev_idx,
  Key prev_key, 
  double prev_bpm,
  double cost,
  double& best_cost,
  int max_len,
  Tracks& best
  )
{
  // We're too long!
  if (chosen.size() >= static_cast<size_t>(max_len)) {
    return;
  }

  // We have a new best length (more important than cost)
  if (chosen.size() > best.size()) {
    best_cost = cost;
    best = chosen;
    std::cout << "New best of length " << best.size() << " found." << std::endl;
  } 
  // Mix length is the same, but less cost
  else if (chosen.size() == best.size() && cost < best_cost && !chosen.empty()) {
    best_cost = cost;
    std::cout << "New best cost of " << best_cost << " found." << std::endl;
    best = chosen;
  }

  // Nothing to look at
  if (available.empty()) {
    std::cout << "No more tracks to choose." << std::endl;
    return;
  }

  // If we haven't chosen anything, all available tracks are candidates
  if (chosen.empty()) {
    for (size_t i = 0; i < available.size(); ++i) {
      Tracks now_available = available;
      Track t = now_available[i];
      now_available.erase(now_available.begin() + i);
      Tracks now_chosen = chosen;
      now_chosen.push_back(t);
      std::cout << "Starting with " << names[t.idx]<< std::endl;
      ChooseTrack(names, now_available, now_chosen, t.idx, t.key, t.bpm, cost, best_cost, max_len, best);
    }
  }
  // If we've chosen something, the track we choose must be compatible with the last one
  else {
    Track prev(prev_idx, prev_bpm, prev_key);

    // Go through all available and find those compatible
    Tracks candidates;
    Keys candidates_adj;
    std::vector<double> candidate_costs;

    for (auto t : available) {
      Key candidate_adj;
      double this_cost;
      if (AreCompatibleTracks(
        prev, 
        t, 
        kBPMThresh, 
        kKeyShiftThresh,
        candidate_adj,
        this_cost
        )) {
        candidates.push_back(t);
        candidates_adj.push_back(candidate_adj);
        candidate_costs.push_back(this_cost);
      }
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
      Tracks now_available = available;
      Track t = candidates[i];
      
      // Erase the candidate from the now available list
      for (size_t j = 0; j < now_available.size(); ++j) {
        if (now_available[j] == t) {
          now_available.erase(now_available.begin() + j);
          break;
        }
      }

      // Set to our new key
      t.key = candidates_adj[i];

      Tracks now_chosen = chosen;
      now_chosen.push_back(t);

      // NOTE: We send in an ADJUSTED key for this track!
      ChooseTrack(names, now_available, now_chosen, t.idx, t.key, t.bpm, cost + candidate_costs[i], best_cost, max_len, best);
    }
  }
}

int main(int argc, char* argv[])
{
  RunTests();

  // Our tracks
  Tracks tracks;
  std::vector<std::string> names;
  //GetTracks("tracks.txt", tracks);
  GetTracksTabSeparated("tracks_tsv.txt", tracks, names);

  //// Write separate values out
  //std::ofstream names("names.txt");
  //std::ofstream bpms("bpms.txt");
  //std::ofstream keys("keys.txt");
  //for (auto t : tracks) {
  //  names << t.name           << std::endl;
  //  bpms  << t.bpm            << std::endl;
  //  keys  << t.key.short_name << std::endl;
  //}
  //names.close();
  //bpms.close();
  //keys.close();

  // Collect stats on how many tracks are in which keys
  KeyCount key_counts;
  for (auto t : tracks) {
    //key_counts[t.key].push_back(t);
    key_counts[t.key]++;
  }

  // Write the key counts
  for (auto k : key_counts) {
    std::cout << Key::GetShortName(k.first.num, k.first.type) << ": " << k.second << std::endl;
  }

  // Exhaustive solution
  // Start at each track and try to get as many tracks into a mix as possible
  // We will exhaustively try to join into each possible next track that is compatible
  Tracks available = tracks;
  Tracks chosen;
  Tracks best;
  double cost = 0;
  double best_cost = DBL_MAX;
  ChooseTrack(names, available, chosen, -1, Key(), 0, cost, best_cost, kMixSongLen, best);

  // Show our results
  for (auto t : best) {
    std::cout 
      << std::setw(3) << Key::GetShortName(t.key.num, t.key.type) << " @ " 
      << std::setw(3) << static_cast<int>(t.bpm) << "bpm" 
      << ", " << names[t.idx] << std::endl;
  }
  return EXIT_SUCCESS;

  //// Sort by BPM
  //std::sort(tracks.begin(), tracks.end(), [](Track const& a, Track const& b)
  //{
  //  return a.bpm < b.bpm;
  //});

  //// Interactive selection
  //std::vector<Track> order;
  //int idx = 0;

  //for (;;) {

  //  // Mark our work
  //  Track selected = tracks[idx];
  //  tracks.erase(tracks.begin() + idx);
  //  order.push_back(selected);

  //  std::cout << "Your current track: " << selected.name << std::endl;

  //  // Get options...
  //  std::vector<TrackOption> options;
  //  for (size_t i = 0; i < tracks.size(); ++i) {
  //    // Must be compatible
  //    if (!AreCompatibleKeys(selected.key, tracks[i].key)) {
  //      continue;
  //    }

  //    // Get the BPM ratio
  //    double bpm_ratio = std::max(selected.bpm, tracks[i].bpm) / std::min(selected.bpm, tracks[i].bpm);
  //    //double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

  //    // We have an option
  //    TrackOption option(tracks[i], bpm_ratio, i);
  //    options.push_back(option);
  //  }

  //  // Sort by BPM ratio
  //  std::sort(options.begin(), options.end(), [](TrackOption const& a, TrackOption const& b) 
  //  {
  //    return a.bpm_ratio < b.bpm_ratio;
  //  }
  //  );

  //  // Print options
  //  std::cout << "Options: " << std::endl;
  //  for (size_t i = 0; i < options.size(); ++i) {
  //    std::cout << i << "\t" << std::fixed << std::setw(4) << options[i].bpm_ratio << "\t" << options[i].track.name << " (" << options[i].track.key.short_name << ")" << std::endl;
  //  }

  //  std::cout << "You have selected " << order.size() << " tracks" << std::endl;

  //  std::string choice;
  //  std::getline(std::cin, choice);

  //  if (!choice.compare("q")) {
  //    break;
  //  }

  //  std::stringstream ss(choice);
  //  ss >> idx;
  //  idx = options[idx].idx;
  //}

  //// Write our order
  //std::ofstream order_f("order.txt");
  //for (auto t : order) {
  //  order_f << t.bpm << "\t" << t.name << " (" << t.key.short_name << ")" << std::endl;
  //}
  //order_f.close();

  //// Start by just choosing a base track (try each one)
  //std::vector<Tracks> options;
  //// Our starting track
  //for (auto t : tracks) {
  //  std::cout << "Starting with " << t.name << std::endl;
  //  Tracks order;
  //  if (ChooseTrack(t, tracks, order)) {
  //    options.push_back(order);
  //    std::cout << "Found " << options.size() << " solutions!" << std::endl;
  //  }
  //}

  //// Select a key ordering
  //std::vector<Keys> options;
  //int max_depth = 0;
  //for (auto k : key_counts) {
  //  std::cout << "Starting with " << k.first.short_name << std::endl;
  //  Keys order;
  //  if (ChooseKey(KeyFromString("Dbm") /*k.first*/, key_counts, order, 1, max_depth)) {
  //    options.push_back(order);
  //    std::cout << "Found " << options.size() << " solutions!" << std::endl;
  //  }
  //}

  // Find a mix that works!
  MixAnt ma;
  Mix m = ma.FindMix(tracks);

  // Print the mix
  //std::cout << m << std::endl;

  std::cout << "Mix uses " << m.steps.size() << " of " << tracks.size() << " input tracks" << std::endl << std::endl;

  // Save the mix to a file
  std::ofstream ofs("mix.txt");
  ofs << m;
  ofs.close();

  // write our unused tracks
  Tracks unused(tracks);
  for (auto s : m.steps) {
    for (auto it = unused.begin(); it != unused.end(); ++it) {
      if (*it == s.track) {
        unused.erase(it);
        break;
      }
    }
  }
    
  std::ofstream unused_f("unused.txt");
  for (auto u : unused) {
    //unused_f << u.name << std::endl;
    //unused_f << u.key.short_name << std::endl;
    unused_f << u.bpm << std::endl;
  }
  unused_f.close();

  //std::cin.get();
}
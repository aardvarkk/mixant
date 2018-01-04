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

using namespace std;

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

//typedef map< Key, vector<Track> > KeyCount
typedef set<Key> KeySet;
typedef map<Key, int> KeyCount;

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
    cout << "New max depth: " << max_depth << endl;
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
//  now_available.erase(find(now_available.begin(), now_available.end(), track));
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

void GetTracks(string const& path, Tracks& tracks, vector<string>& names)
{
  tracks.clear();

  // Read the file to get our tracks
  ifstream ifs(path);
  string line;
  int idx = 0;
  while (getline(ifs, line)) {
    string name = line;

    getline(ifs, line);
    Key key = Key::KeyFromString(line);

    double bpm;
    getline(ifs, line);
    stringstream ss(line);
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

void GetTracksTabSeparated(string const& path, Tracks& tracks, vector<string>& names)
{
  tracks.clear();

  // Read the file to get our tracks
  ifstream ifs(path);
  string line;
  int idx = 0;
  while (getline(ifs, line)) {
    stringstream ss(line);
    
    string name;
    getline(ss, name, '\t');

    string key_str;
    getline(ss, key_str, '\t');
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
  auto min_bpm = min(t1.bpm, t2.bpm);
  auto max_bpm = max(t1.bpm, t2.bpm);
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
  vector<string> names,
  Tracks& available, 
  Tracks& chosen, 
  int prev_idx,
  Key prev_key, 
  double prev_bpm,
  double cost,
  double& best_cost,
  int max_len,
  Tracks& best,
  int& its
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
    cout << "New best of length " << best.size() << " found." << endl;
    its = 0;
  } 
  // Mix length is the same, but less cost
  else if (chosen.size() == best.size() && cost < best_cost && !chosen.empty()) {
    best_cost = cost;
    cout << "New best cost of " << best_cost << " found." << endl;
    best = chosen;
    its = 0;
  }
  // No improvement!
  else {
    ++its;
  }

  // We've spent too long, so bail!
  if (its >= 1000000) {
    cout << "Too many iterations without improvement!" << endl;
    return;
  }

  // Nothing to look at
  if (available.empty()) {
    cout << "No more tracks to choose." << endl;
    return;
  }

  Track prev(prev_idx, prev_bpm, prev_key);

  // Go through all available and find those compatible
  Tracks candidates;
  Keys candidates_adj;
  vector<double> candidate_costs;

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
    ChooseTrack(names, now_available, now_chosen, t.idx, t.key, t.bpm, cost + candidate_costs[i], best_cost, max_len, best, its);
  }
}

int main(int argc, char* argv[])
{
  RunTests();

  // Our tracks
  Tracks tracks;
  vector<string> names;
  //GetTracks("tracks.txt", tracks);
  GetTracksTabSeparated("tracks_tsv.txt", tracks, names);

  //// Write separate values out
  //ofstream names("names.txt");
  //ofstream bpms("bpms.txt");
  //ofstream keys("keys.txt");
  //for (auto t : tracks) {
  //  names << t.name           << endl;
  //  bpms  << t.bpm            << endl;
  //  keys  << t.key.short_name << endl;
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
    cout << Key::GetShortName(k.first.num, k.first.type) << ": " << k.second << endl;
  }

  // Exhaustive solution
  // Start at each track and try to get as many tracks into a mix as possible
  // We will exhaustively try to join into each possible next track that is compatible
  Tracks available = tracks;
  Tracks best;
  double best_cost = DBL_MAX;
  for (size_t i = 0; i < available.size(); ++i) {
    Tracks chosen;
    Tracks this_best;
    double cost = 0;
    double this_best_cost = DBL_MAX;

    int its = 0;
    Tracks now_available = available;
    Track t = now_available[i];
    now_available.erase(now_available.begin() + i);
    Tracks now_chosen = chosen;
    now_chosen.push_back(t);
    cout << "Starting with " << names[t.idx] << endl;
    ChooseTrack(names, now_available, now_chosen, t.idx, t.key, t.bpm, cost, this_best_cost, kMixSongLen, this_best, its);

    if (this_best.size() > best.size()) {
      best = this_best;
      cout << "New OVERALL best of " << best.size() << " found!" << endl;
    }
    else if (this_best.size() == best.size() && this_best_cost < best_cost) {
      best_cost = this_best_cost;
      best = this_best;
      cout << "New OVERALL best cost of " << best_cost << " found!" << endl;
    }
  }

  // Show our results
  for (auto t : best) {
    cout
      << setw(3) << Key::GetShortName(t.key.num, t.key.type) << " @ "
      << setw(3) << static_cast<int>(t.bpm) << "bpm"
      << ", " << names[t.idx] << endl;
  }
  cout << "Cost is " << best_cost << endl;

  return EXIT_SUCCESS;

  //// Sort by BPM
  //sort(tracks.begin(), tracks.end(), [](Track const& a, Track const& b)
  //{
  //  return a.bpm < b.bpm;
  //});

  //// Interactive selection
  //vector<Track> order;
  //int idx = 0;

  //for (;;) {

  //  // Mark our work
  //  Track selected = tracks[idx];
  //  tracks.erase(tracks.begin() + idx);
  //  order.push_back(selected);

  //  cout << "Your current track: " << selected.name << endl;

  //  // Get options...
  //  vector<TrackOption> options;
  //  for (size_t i = 0; i < tracks.size(); ++i) {
  //    // Must be compatible
  //    if (!AreCompatibleKeys(selected.key, tracks[i].key)) {
  //      continue;
  //    }

  //    // Get the BPM ratio
  //    double bpm_ratio = max(selected.bpm, tracks[i].bpm) / min(selected.bpm, tracks[i].bpm);
  //    //double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

  //    // We have an option
  //    TrackOption option(tracks[i], bpm_ratio, i);
  //    options.push_back(option);
  //  }

  //  // Sort by BPM ratio
  //  sort(options.begin(), options.end(), [](TrackOption const& a, TrackOption const& b) 
  //  {
  //    return a.bpm_ratio < b.bpm_ratio;
  //  }
  //  );

  //  // Print options
  //  cout << "Options: " << endl;
  //  for (size_t i = 0; i < options.size(); ++i) {
  //    cout << i << "\t" << fixed << setw(4) << options[i].bpm_ratio << "\t" << options[i].track.name << " (" << options[i].track.key.short_name << ")" << endl;
  //  }

  //  cout << "You have selected " << order.size() << " tracks" << endl;

  //  string choice;
  //  getline(cin, choice);

  //  if (!choice.compare("q")) {
  //    break;
  //  }

  //  stringstream ss(choice);
  //  ss >> idx;
  //  idx = options[idx].idx;
  //}

  //// Write our order
  //ofstream order_f("order.txt");
  //for (auto t : order) {
  //  order_f << t.bpm << "\t" << t.name << " (" << t.key.short_name << ")" << endl;
  //}
  //order_f.close();

  //// Start by just choosing a base track (try each one)
  //vector<Tracks> options;
  //// Our starting track
  //for (auto t : tracks) {
  //  cout << "Starting with " << t.name << endl;
  //  Tracks order;
  //  if (ChooseTrack(t, tracks, order)) {
  //    options.push_back(order);
  //    cout << "Found " << options.size() << " solutions!" << endl;
  //  }
  //}

  //// Select a key ordering
  //vector<Keys> options;
  //int max_depth = 0;
  //for (auto k : key_counts) {
  //  cout << "Starting with " << k.first.short_name << endl;
  //  Keys order;
  //  if (ChooseKey(KeyFromString("Dbm") /*k.first*/, key_counts, order, 1, max_depth)) {
  //    options.push_back(order);
  //    cout << "Found " << options.size() << " solutions!" << endl;
  //  }
  //}

  // Find a mix that works!
  MixAnt ma;
  Mix m = ma.FindMix(tracks);

  // Print the mix
  //cout << m << endl;

  cout << "Mix uses " << m.steps.size() << " of " << tracks.size() << " input tracks" << endl << endl;

  // Save the mix to a file
  ofstream ofs("mix.txt");
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
    
  ofstream unused_f("unused.txt");
  for (auto u : unused) {
    //unused_f << u.name << endl;
    //unused_f << u.key.short_name << endl;
    unused_f << u.bpm << endl;
  }
  unused_f.close();

  //cin.get();
}
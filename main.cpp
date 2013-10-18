#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "camelot.h"
#include "mixant.h"
#include "track.h"
#include "utils.h"

struct TrackOption
{
  Track track;
  double bpm_ratio;
  int idx;

  TrackOption(Track const& track, double bpm_ratio, int idx) : track(track), bpm_ratio(bpm_ratio), idx(idx) {}
};

//typedef std::map< Camelot::Key, std::vector<Track> > KeyCount
typedef std::set<Camelot::Key> KeySet;
typedef std::map<Camelot::Key, int> KeyCount;

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

bool ChooseKey(Camelot::Key const& key, KeyCount const& key_count, Camelot::Keys order, int depth, int& max_depth)
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
  Camelot::Keys compatible = Camelot::GetCompatibleKeys(key);
  for (auto k : compatible) {
    if (new_counts.find(k) != new_counts.end()) {
      ChooseKey(k, new_counts, order, depth + 1, max_depth);
      found_compatible = true;
    }
  }

  // Dead end solution...
  return found_compatible;
}

bool ChooseTrack(Track const& track, Tracks const& available, Tracks order)
{
  // We've run out of tracks!
  // We've found a potential solution!
  if (available.empty()) {
    return true;
  }

  // Add it to our order, but it's no longer available
  order.push_back(track);

  // Make our own copy to iterate through...
  Tracks now_available(available);
  
  // Erase the one we just took
  now_available.erase(std::find(now_available.begin(), now_available.end(), track));

  // Find all compatible tracks, then try to choose each one in turn
  bool found_compatible = false;
  for (auto t : now_available) {
    if (Camelot::AreCompatibleKeys(track.key, t.key)) {
      ChooseTrack(t, now_available, order);
      found_compatible = true;
    }
  }

  // Dead end solution...
  return found_compatible;
}

void GetTracks(std::string const& path, Tracks& tracks)
{
  tracks.clear();

  // Read the file to get our tracks
  std::ifstream ifs("tracks.txt");
  std::string line;
  while (std::getline(ifs, line)) {
    std::string name = line;

    std::getline(ifs, line);
    Camelot::Key key = Camelot::KeyFromString(line);

    double bpm;
    std::getline(ifs, line);
    std::stringstream ss(line);
    ss >> bpm;

    // Allow commenting out tracks
    if (!name.substr(0, 2).compare("//")) {
      continue;
    }

    tracks.push_back(Track(name, bpm, key));
  }
  ifs.close();
}

int main(int argc, char* argv[])
{
  //// TESTS
  //Camelot::Key Am = Camelot::KeyFromString("Am");
  //Camelot::Key F = Camelot::KeyFromString("F");
  //int d = Camelot::GetCamelotDistance(Am, F); // should be 2

  //Camelot::Key Fm = Camelot::KeyFromString("Fm");
  //Camelot::Key Eb = Camelot::KeyFromString("Eb");
  //bool c = Camelot::AreCompatibleKeys(Fm, Eb); // should be false

  // Our tracks
  Tracks tracks;
  GetTracks("tracks.txt", tracks);

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
    std::cout << k.first.short_name << ": " << k.second << std::endl;
  }

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
  //    if (!Camelot::AreCompatibleKeys(selected.key, tracks[i].key)) {
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

  //// Write our unused tracks
  //std::ofstream unused_f("unused.txt");
  //std::vector<Track> unused;
  //for (auto t : tracks) {
  //  unused_f << t.name << std::endl;
  //  unused_f << t.key.short_name << std::endl;
  //  unused_f << t.bpm << std::endl;
  //}
  //unused_f.close();

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
  //std::vector<Camelot::Keys> options;
  //int max_depth = 0;
  //for (auto k : key_counts) {
  //  std::cout << "Starting with " << k.first.short_name << std::endl;
  //  Camelot::Keys order;
  //  if (ChooseKey(Camelot::KeyFromString("Dbm") /*k.first*/, key_counts, order, 1, max_depth)) {
  //    options.push_back(order);
  //    std::cout << "Found " << options.size() << " solutions!" << std::endl;
  //  }
  //}

  // Find a mix that works!
  MixAnt ma;
  double min_dist;
  Mix m = ma.FindMix(tracks, &min_dist);

  // Print the mix
  //std::cout << m << std::endl;

  // Save the mix to a file
  std::ofstream ofs("mix.txt");
  ofs << "Score: " << min_dist << std::endl << std::endl;
  ofs << m;
  ofs.close();

  //std::cin.get();
}
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
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

int main(int argc, char* argv[])
{
  // TESTS
  Camelot::Key Am = Camelot::KeyFromString("Am");
  Camelot::Key F = Camelot::KeyFromString("F");
  int d = Camelot::GetCamelotDistance(Am, F); // should be 2

  Camelot::Key Fm = Camelot::KeyFromString("Fm");
  Camelot::Key Eb = Camelot::KeyFromString("Eb");
  bool c = Camelot::AreCompatibleKeys(Fm, Eb); // should be false

  // Our tracks
  Tracks tracks;

  // Read the file to get our tracks
  std::ifstream ifs("unused10.txt");
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

  // Write separate values out
  std::ofstream names("names.txt");
  std::ofstream bpms("bpms.txt");
  std::ofstream keys("keys.txt");
  for (auto t : tracks) {
    names << t.name           << std::endl;
    bpms  << t.bpm            << std::endl;
    keys  << t.key.short_name << std::endl;
  }
  names.close();
  bpms.close();
  keys.close();

  // Collect stats on how many tracks are in which keys
  std::map< Camelot::Key, std::vector<Track> > key_counts;
  for (auto t : tracks) {
    key_counts[t.key].push_back(t);
  }

  // Write the key counts
  std::ofstream key_counts_f("key_counts.txt");
  for (auto k : key_counts) {
    key_counts_f << k.first.short_name << "\t" << k.second.size() << std::endl << std::endl;
    for (auto t : k.second) {
      key_counts_f << t.name << std::endl;
    }
    key_counts_f << std::endl;
  }
  key_counts_f.close();

  // Sort by BPM
  std::sort(tracks.begin(), tracks.end(), [](Track const& a, Track const& b)
  {
    return a.bpm < b.bpm;
  });

  // Interactive selection
  std::vector<Track> order;
  int idx = 0;

  for (;;) {

    // Mark our work
    Track selected = tracks[idx];
    tracks.erase(tracks.begin() + idx);
    order.push_back(selected);

    std::cout << "Your current track: " << selected.name << std::endl;

    // Get options...
    std::vector<TrackOption> options;
    for (size_t i = 0; i < tracks.size(); ++i) {
      // Must be compatible
      if (!Camelot::AreCompatibleKeys(selected.key, tracks[i].key)) {
        continue;
      }

      // Get the BPM ratio
      double bpm_ratio = std::max(selected.bpm, tracks[i].bpm) / std::min(selected.bpm, tracks[i].bpm);
      //double bpm_ratio_st = log(bpm_ratio) / log(Utils::GetSemitoneRatio());

      // We have an option
      TrackOption option(tracks[i], bpm_ratio, i);
      options.push_back(option);
    }

    // Sort by BPM ratio
    std::sort(options.begin(), options.end(), [](TrackOption const& a, TrackOption const& b) 
    {
      return a.bpm_ratio < b.bpm_ratio;
    }
    );

    // Print options
    std::cout << "Options: " << std::endl;
    for (size_t i = 0; i < options.size(); ++i) {
      std::cout << i << "\t" << std::fixed << std::setw(4) << options[i].bpm_ratio << "\t" << options[i].track.name << " (" << options[i].track.key.short_name << ")" << std::endl;
    }

    std::cout << "You have selected " << order.size() << " tracks" << std::endl;

    std::string choice;
    std::getline(std::cin, choice);

    if (!choice.compare("q")) {
      break;
    }

    std::stringstream ss(choice);
    ss >> idx;
    idx = options[idx].idx;
  }

  // Write our order
  std::ofstream order_f("order.txt");
  for (auto t : order) {
    order_f << t.bpm << "\t" << t.name << " (" << t.key.short_name << ")" << std::endl;
  }
  order_f.close();

  // Write our unused tracks
  std::ofstream unused_f("unused.txt");
  std::vector<Track> unused;
  for (auto t : tracks) {
    unused_f << t.name << std::endl;
    unused_f << t.key.short_name << std::endl;
    unused_f << t.bpm << std::endl;
  }
  unused_f.close();

  //// Find a mix that works!
  //MixAnt ma;
  //Mix m = ma.FindMix(tracks);

  //// Print the mix
  ////std::cout << m << std::endl;

  //// Save the mix to a file
  //std::ofstream ofs("mix.txt");
  //ofs << m;
  //ofs.close();

  //std::cin.get();
}
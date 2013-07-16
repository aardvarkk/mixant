#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "camelot.h"
#include "mixant.h"
#include "track.h"

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
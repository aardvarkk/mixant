#include <fstream>
#include <iostream>
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

  // Find a mix that works!
  MixAnt ma;
  Mix m = ma.FindMix(tracks);

  // Print the mix
  //std::cout << m << std::endl;

  // Save the mix to a file
  std::ofstream ofs("mix.txt");
  ofs << m;
  ofs.close();

  std::cin.get();
}
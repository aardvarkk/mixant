#include <fstream>
#include <sstream>

#include "camelot.h"
#include "mixant.h"
#include "track.h"

int main(int argc, char* argv[])
{
  // Our tracks
  Tracks tracks;

  // Read the file to get our tracks
  std::ifstream ifs("tracks.txt");
  std::string line;
  while (std::getline(ifs, line)) {
    std::string name = line;
    
    double bpm;
    std::getline(ifs, line);
    std::stringstream ss(line);
    ss >> bpm;

    std::getline(ifs, line);
    Camelot::Key key = Camelot::KeyFromString(line);

    tracks.push_back(Track(name, bpm, key));
  }
  ifs.close();

  // Find a mix that works!
  MixAnt ma;
  Mix m = ma.FindMix(tracks);
}